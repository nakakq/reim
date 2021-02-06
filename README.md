# ReIm: A Real-time Implementation of WORLD-like Vocoder

ReIm is a speech analysis/synthesis system that supports real-time analysis and synthesis. 




## Overview

ReIm expresses voice using 3 acoustic features; fundamental frequency (Fo), aperiodicity (Ap), and spectral envelope (Sp). The parameters are compatible with the C++ implementation of WORLD. 

Core part of ReIm is written in C. It requires the ISO C99 features (e.g. `stdint.h`, `stdbool.h`) at the minimum. 



## Usage

See the [example/example.c](./example/example.c). 



## How to Build

Clone this repository with the submodules. 

```
$ git clone --recursive https://github.com/nakakq/reim
```

Required external dependencies are following; I'm planning to make these unnessesary in a future. 

- libsndfile
- PortAudio



On Linux / WSL (Windows Subsystem for Linux) / MSYS2 environment, you can use `make ` to build. 

- `make lib`: Build the library. 
- `make run`: Build and run the example. 
- `make test`: Build and run the tests. 
- `make memcheck`: Check memory leaks with [Valgrind](https://valgrind.org/). (for developers)

For Visual Studio 2019 users, the solution file is available in the `vs2019` directory (*NOTE: it is currently placeholder*). 

If you want to use external FFT packages: 

- Intel Math Kernel Library:
  Set your include path, link the library file, and define a preprocessor macro `REIM_USE_MKL` using your compiler's option. 
- FFTW 3:
  Set your include path, link the library file, and define a preprocessor macro `REIM_USE_FFTW3` using your compiler's option. 



## Notes for WORLD users

The algorithms are based on WORLD but are modified to perform in real-time. The major changes are the following: 

- ReIm's typical analysis procedure is  `Silence → Fo → Ap → Sp`. 
- ReIm provides silence detection feature. It may reduce the computation during the silence. 
- The Fo analyzer does not provide the voiced/unvoiced (VUV) decision. Instead, the aperiodicity analysis does. 
- The recommended frame period is around 5.0 ms. A longer frame period may cause some artifacts because the synthesizer doesn't interpolate the neighboring frames. 



### The Algorithm

- Fo analyzer is based on Distributed Inline Operation (DIO) and Summation of Residual Harmonics (SRH). First, the Fo candidates are extracted by DIO with zero-crossing. Then, they are refined with the instantaneous frequency. Finally, the best Fo is chosen by the SRH score from the candidates. 
- Ap analyzer is currently not implemented. 
- Sp analyzer is mostly equivalent to CheapTrick except for the unvoiced processing. 
- Synthesizer is also similar to the WORLD's. Velvet noise is used for the aperiodic excitation. 



## License

ReIm is released under the [MIT license](./LICENSE). 

### Acknowledgments

- The vocoder design/algorithm is based on [WORLD](https://github.com/mmorise/World). 
- The default FFT implementation (in [fftsg.c](src/reim/fftsg.c)) is from [General Purpose FFT Package](http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html). 
- For testing, ReIm uses [doctest](https://github.com/onqtam/doctest) (a single-header C++ testing framework). 



## References

Fo analysis: 

- 森勢 将雅, 河原 英紀, 西浦 敬信: "基本波検出に基づく高SNRの音声を対象とした高速なF0推定法," 電子情報通信学会 論文誌D, vol. J93-D, no. 2, pp. 109–117, 2010. 
- T. Drugman and A. Alwan: "Joint Robust Voicing Detection and Pitch Estimation Based on Residual Harmonics", *Interspeech '11*, 2011. 

Aperiodicity analysis: 

- M. Morise: "D4C, a band-aperiodicity estimator for high-quality speech synthesis," *Speech Communication*, vol. 84, pp. 57–65, 2016. 

Spectral envelope analysis: 

- M. Morise: "CheapTrick, a spectral envelope estimator for high-quality speech synthesis," *Speech Communication*, vol. 67, pp. 1–7, 2015. 

