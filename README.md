<div align="center">
<h1>ACV-tree </h1>
<div align="left">

* 💻 [Github Link](https://github.com/yuquanle/Sentence-similarity-modeling)
* 📄 [Conference Paper Link (IJCAI 2018)](https://www.ijcai.org/proceedings/2018/0575.pdf)
* 📄 [Journal Paper Link (TALSP 2019)](https://ieeexplore.ieee.org/abstract/document/8642425/)
* 📁 [Datasets Link](https://github.com/facebookresearch/SentEval)


## Project Overview
Sentence similarity modeling lies at the core of many natural language processing applications, and thus has received much attention. Owing to the success of word embeddings, recently, popular neural network methods have achieved sentence embedding, obtaining attractive performance. Nevertheless, most of them focused on learning semantic information and modeling it as a continuous vector, while the syntactic information of sentences has not been fully exploited. On the other hand, prior works have shown the benefits of structured trees that include syntactic information, while few methods in this branch utilized the advantages of word embeddings and another powerful technique—attention weight mechanism. This paper makes the first attempt to absorb their advantages by merging these techniques in a unified structure, dubbed as ACV-tree. Meanwhile, this paper develops a new tree kernel, known as ACVT kernel, that is tailored for sentence similarity measure based on the proposed structure. The experimental results, based on 19 widely-used datasets, demonstrate that our model is effective and competitive, compared against state-of-the-art models.Additionally, the experimental results validate that many attention weigh mechanisms and word embedding techniques can be seamlessly integrated into our model, demonstrating the
robustness and universality of our model.

## Usage Instructions
```
./demo.sh
```

## Citation
If you find this code useful for your research, please consider citing the following papers:
```
@article{le2018acv,
  title={ACV-tree: A New Method for Sentence Similarity Modeling.},
  author={Le, Yuquan and Wang, Zhi-Jie and Quan, Zhe and He, Jiawei and Yao, Bin},
  journal={International Joint Conference on Artificial Intelligence (IJCAI)},
  pages={4137--4143},
  year={2018}
}

@article{quan2019efficient,
  title={An efficient framework for sentence similarity modeling},
  author={Quan, Zhe and Wang, Zhi-Jie and Le, Yuquan and Yao, Bin and Li, Kenli and Yin, Jian},
  journal={IEEE/ACM Transactions on Audio, Speech, and Language Processing},
  volume={27},
  number={4},
  pages={853--865},
  year={2019}
}
```

## Acknowledgements
This project is based on [usvmtk](https://ikernels-portal.disi.unitn.it/repository/usvmtk/). We thank them for their outstanding work.
