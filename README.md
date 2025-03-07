# Practice code for CSC43043EP - 3D Graphics

## Download



### Clone

```
git clone --recurse-submodules https://github.com/drohmer/csc43043ep_practice_code.git
```

or

```
git clone https://github.com/drohmer/csc43043ep_practice_code.git
cd csc43043ep_practice_code/
git submodule init
git submodule update
```

### Download archive

* [csc43043ep_practice_code.zip](https://graphicscomputing.fr/course/2024_2025/CSC_43043_EP/lab_code/csc43043ep_practice_code.zip)
* [csc43043ep_practice_code.tar.gz](https://graphicscomputing.fr/course/2024_2025/CSC_43043_EP/lab_code/csc43043ep_practice_code.tar.gz)


_Note: Do not use the "Code -> Download ZIP" option from the GitHub GUI as you won't get the CGP library as submodule with the code._


## Compilation

Scenes are in a directory _scenes_csc43043ep/xx_Name/_

Each scene is provided with its own CMakeLists.txt and Makefile

Compilation via CMake
```
cd scenes_csc43043ep/01_introduction/
cmake -B build/
cd build/
make -j$(nproc)
./01_introduction
```

Compilation via Makefile
```
cd scenes_csc43043ep/01_introduction/
make -j$(nproc)
./01_introduction
```

More details are provided in the dedicated [Installation setup and Compilation page](https://imagecomputing.net/cgp/compilation) if needed (Windows, Linux, Mac).

## Requirement

The code requires the GLFW library in Linux.

ex. on Ubuntu

```
sudo apt-get install libglfw3-dev
```



## Links

* [Lab class exercises](https://graphicscomputing.fr/course/2024_2025/CSC_43043_EP/)
* [Documentation on CGP library](https://imagecomputing.net/cgp/index.html)

