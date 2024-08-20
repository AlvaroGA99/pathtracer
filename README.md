# Pathtracer
Unidirectional path tracer based on educational pathtracer codebase Nori by Wenzel Jakob. In order to acomplish author's decissions over copyright and distribution, this repository only contains the modified files that were needed to implement the feautres of the pathtracer.

The pathtracer makes use of diferent Integrator class files to implement the four different versions of the pathtracer, wich are the following ones.

* direct.cpp : Direct lighting featuring Multiple Importance Sampling.
* path.cpp : Indirect lighting using russian roulette method to finish paths using luminance as the probability factor.
* path_nee : Same as path.cpp but implementing both Direct and Indirect lighting by Next Event Estimation .
* path_nee_dof : Same version as path_nee with a depth of field effect.

Some of the results are shown below 

600 samples using path.cpp

500 samples using path_nee.cpp

600 samples with path_nee_dof (10 samples of secondary rays and 0.1 aperture of the camera shutter, and at a focal distance of 5)
