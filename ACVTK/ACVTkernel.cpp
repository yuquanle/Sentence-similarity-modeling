/***********************************************************************/
/*   ACV Tree Kernel                                                   */
/*                                                                     */
/*   ACVTkernel.cpp                                                    */
/*                                                                     */
/*   This source is modified by Fast Tree kernel                       */
/*                                                                     */
/*   Author: Yuquan Le, joewellHe 				                       */
/*   leyuquan@hnu.edu.cn, hejiawei@hnu.edu.cn                          */	
/*   Date: 2018-1-15                                                   */
/***********************************************************************/

#define _CRT_SECURE_NO_WARNINGS

//#define TEST(a,b) printf("The value of " #a " = " #b "\n", a)
#define DPS(i,j) (*(DPS+(i)*(m+1)+j))
#define DP(i,j) (*(DP+(i)*(m+1)+j))


#include "svm_common.h"
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <ctype.h>
#include <algorithm>
#include <iostream>

double LAMBDA2;
double LAMBDA;
double SIGMA;
double MU;
double REMOVE_LEAVES = 0; // if equal to MU*LAMBDA2, it removes the leaves contribution;

short  TKGENERALITY;  //store the generality of the kernel PT is 2 whereas SST and ST is 1
                      // used to load the opportune data structures (with SST and ST uses faster approach)
short  PARAM_VECT; // if 1 the vector of parameters is defined

double delta_matrix[MAX_NUMBER_OF_NODES][MAX_NUMBER_OF_NODES];
// local functions


void determine_sub_lists(FOREST *a, FOREST *b, nodePair *intersect, int *n);
double Delta_SK(TreeNode **Sx, TreeNode ** Sz, int n, int m); // delta over children
double Delta_PT( TreeNode * Nx, TreeNode * Nz); // Delta for PT

void evaluateNorma(KERNEL_PARM * kernel_parm, DOC * d); // evaluate norm of trees and vectors
//------------------------
void getAllWeightToMmap(TreeNode* tree, map<string,double> *weight_map);
void getWordAttn(char* sName, char* word, char* weight);
double SIM(double* a, double* b, int size);

//int ReadEmbedding(const char *file_name);

#ifdef FAST

double Delta_SK(TreeNode **Sx, TreeNode ** Sz, int n, int m){
	double DPS[MAX_NUMBER_OF_CHILDREN_PT][MAX_NUMBER_OF_CHILDREN_PT];
	double DP[MAX_NUMBER_OF_CHILDREN_PT][MAX_NUMBER_OF_CHILDREN_PT];
	double kernel_mat[MAX_NUMBER_OF_CHILDREN_PT];
  
	int i,j,l,p;
	double K;
  
  
	p = n;
	if (m<n) 
	  p=m;
	if (p>MAX_CHILDREN) 
	  p=MAX_CHILDREN;
    
	for (j=0; j<=m; j++)
		for (i=0; i<=n; i++)
			DPS[i][j]=DP[i][j]=0;
	
	kernel_mat[0]=0;
	for (i=1; i<=n; i++)
		for (j=1; j<=m; j++)
			
			if((strcmp((*(Sx+i-1))->sName,(*(Sz+j-1))->sName)==0) )
			{
				DPS[i][j]=Delta_PT(*(Sx+i-1),*(Sz+j-1));
				kernel_mat[0]+=DPS[i][j];
			}
			
			else
				DPS[i][j]=0;

	for(l=1;l<p;l++){
		kernel_mat[l]=0;
		for (j=0; j<=m; j++) 
			DP[l-1][j]=0;
		for (i=0; i<=n; i++) 
			DP[i][l-1]=0;
		
		for (i=l; i<=n; i++)
			for (j=l; j<=m; j++){
				DP[i][j] = DPS[i][j]+LAMBDA*DP[i-1][j]
						+ LAMBDA*DP[i][j-1]
						- LAMBDA2*DP[i-1][j-1];
				
				if( (strcmp((*(Sx+i-1))->sName,(*(Sz+j-1))->sName)==0))
				{
				    	DPS[i][j] = Delta_PT(*(Sx+i-1),*(Sz+j-1))* DP[i-1][j-1];
						kernel_mat[l] += DPS[i][j];
				}
				
			}
	}
				
	K=0;
	for(l=0;l<p;l++){
		K+=kernel_mat[l];
    }
    return K;	
	
}

#endif


double Delta_PT( TreeNode * Nx, TreeNode * Nz){
	double sum=0;

	if(delta_matrix[Nx->nodeID][Nz->nodeID]!=-1) {
		return delta_matrix[Nx->nodeID][Nz->nodeID]; // already there
	}

	else if(Nx->iNoOfChildren==0 || Nz->iNoOfChildren==0){
		return delta_matrix[Nx->nodeID][Nz->nodeID];
	}
	else{
		sum = MU*(LAMBDA2+Delta_SK(Nx->pChild, Nz->pChild,Nx->iNoOfChildren, Nz->iNoOfChildren)); 
		return (delta_matrix[Nx->nodeID][Nz->nodeID]=sum);
	}
	return 0;
}



double AVCTreekernel(nodePair *pairs, int n){
	
	int i;
	double sum=0,contr;
	for(i=0;i<n;i++){
		contr=Delta_PT(pairs[i].Nx,pairs[i].Nz);																								 
		sum+=contr;
	}

	return sum;	  
}

void determine_sub_lists(KERNEL_PARM *kernel_parm, FOREST *a, FOREST *b, nodePair *intersect, int *n){
   
   int i=0,j=0,j_old,j_final;
   int n_a,n_b;
   short cfr;
   OrderedTreeNode *list_a, *list_b;

   n_a = a->listSize;
   n_b = b->listSize;
   list_a=a->orderedNodeSet; 
   list_b=b->orderedNodeSet;
   *n=0;

   for(i=0;i<n_a;i++){
	   if(list_a[i].node->pre_terminal==-1){

		   for(j=0;j<n_b;j++){
			   if(list_b[j].node->pre_terminal==-1){ 
				   
				   delta_matrix[list_a[i].node->nodeID][list_b[j].node->nodeID]=0;
				
				   char word1[100];
				   char weight1[100];
				   getWordAttn(list_a[i].node->sName, word1, weight1);
				   
				   char word2[100];
				   char weight2[100];
				   getWordAttn(list_b[j].node->sName, word2, weight2);
				   
				   double attn = atof(weight1)*atof(weight2);
				   if(strcmp(word1,word2)==0){
						intersect[*n].Nx=list_a[i].node;
						intersect[*n].Nz=list_b[j].node;
						(*n)++;
						delta_matrix[list_a[i].node->nodeID][list_b[j].node->nodeID]=attn;
							   
				   }else{
					
						if((dict[word1]!=NULL)&&(dict[word2]!=NULL)){

							double *v1 = dict[word1];
							double *v2 = dict[word2];
							double sim = SIM(v1, v2, 300);
								
							if(sim>0.0){
								intersect[*n].Nx=list_a[i].node;
								intersect[*n].Nz=list_b[j].node;
								(*n)++;
								delta_matrix[list_a[i].node->nodeID][list_b[j].node->nodeID]=sim*attn;					 
							}
						}	
				   }			   
			   }
		 }
	  }else{
		   for(j=0;j<n_b;j++){
			   if(list_b[j].node->pre_terminal!=-1){
				   if(strcmp(list_a[i].sName,list_b[j].sName)==0){
					   intersect[*n].Nx=list_a[i].node;
					   intersect[*n].Nz=list_b[j].node;
					   (*n)++;
					   delta_matrix[list_a[i].node->nodeID][list_b[j].node->nodeID]=-1;
				   }
				   else
					   delta_matrix[list_a[i].node->nodeID][list_b[j].node->nodeID]=0.0;
			   }
		   }
	  }
   }
}

/*Jiawei, return  normalized weigted*/
//map<string,double>
void getAllWeightToMmap(TreeNode* tree, map<string,double> *weight_map){
	if(tree->pre_terminal==-1){
		char word[50], weight[30];
		getWordAttn(tree->sName, word, weight);
		(*weight_map)[word]=atof(weight);
		return;
		}
	int i;
	for(i=0;i<tree->iNoOfChildren;i++){
		getAllWeightToMmap(tree->pChild[i], weight_map);
	}
}

/*Jiawei, compute similarity*/
double SIM(double* a, double* b, int size){
	double sim = 0;
	int i = 0;
	double dotab = 0, norma = 0, normb = 0;
	for(i = 0; i < size; i++){
		dotab += a[i]*b[i];
		norma += a[i]*a[i];
		normb += b[i]*b[i];
	}
	sim = dotab / (sqrt(norma)*sqrt(normb));
	return sim;
}

void getWordAttn(char* sName, char* word, char* weight){
	 int len=strlen(sName);
     int i=0;
	 while(i<len && sName[i]!=':'){
		 word[i]=sName[i];
		 i++;
	 }
	word[i]='\0';
	int j=i+1;
	while(j<len){
	  weight[j-i-1]=sName[j];
	  j++;
	}
	weight[j-i-1]='\0';

}

//-------------------------------------------------------------------------------------------------------
// Author joewellHe 
// General Tree Kernel
// Modified by Thorsten Joachims
//-------------------------------------------------------------------------------------------------------

double tree_kernel_not_norm(KERNEL_PARM * kernel_parm, FOREST * a, FOREST * b){
	
  int n_pairs=0;
  int m = 0;
  int i=0;
  nodePair intersect[MAX_NUMBER_OF_PAIRS];
	
	if(a->orderedNodeSet != NULL && b->orderedNodeSet != NULL) {
		determine_sub_lists(kernel_parm,a,b,intersect,&n_pairs);
	}
	return  AVCTreekernel(intersect,n_pairs);
	
}


void evaluateNorma(KERNEL_PARM * kernel_parm, DOC * d){
	int i;
	double k=0;
	double ks[2]={0,0};
	
	//short kernel_type_tmp=kernel_parm->first_kernel; //save parameters from command line
	double lambda_tmp=LAMBDA; 
	double mu_tmp=MU;
	short TKG_tmp=TKGENERALITY;

	if(d->num_of_trees>=2){  //ensure pair of tree
		if(tree_kernel_params[0].kernel_type==END_OF_TREE_KERNELS)
					PARAM_VECT=0;
					
			k = tree_kernel_not_norm(kernel_parm, d->forest_vec[0], d->forest_vec[1]);
			ks[0] = tree_kernel_not_norm(kernel_parm, d->forest_vec[0], d->forest_vec[0]);
			ks[1] = tree_kernel_not_norm(kernel_parm, d->forest_vec[1], d->forest_vec[1]);
					
			if(k!=0 && (kernel_parm->normalization == 1 || kernel_parm->normalization == 3)) {
				d->forest_vec[0]->twonorm_PT=ks[0]; 
				d->forest_vec[1]->twonorm_PT=ks[1]; 

				k = k / sqrt(d->forest_vec[0]->twonorm_PT * d->forest_vec[1]->twonorm_PT);
			}
			else{
				d->forest_vec[0]->twonorm_PT=1; 
				d->forest_vec[1]->twonorm_PT=1; 
			}
			d->docsim = k;
	//kernel_parm->first_kernel=kernel_type_tmp; //re-set command line parameters 
	LAMBDA=lambda_tmp; 
	MU=mu_tmp;
	TKGENERALITY=TKG_tmp;
    }
} 

