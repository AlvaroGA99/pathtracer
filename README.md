# Pathtracer
Unidirectional path tracer based on educational pathtracer codebase Nori by Wenzel Jakob. In order to acomplish author's decissions over copyright and distribution, this repository only contains the modified files that were needed to implement the feautres of the pathtracer.

The pathtracer makes use of diferent Integrator class files to implement the four different versions of the pathtracer, wich are the following ones.

* direct.cpp : Direct lighting featuring Multiple Importance Sampling.
* path.cpp : Indirect lighting using russian roulette method to finish paths using luminance as the probability factor.
* path_nee : Same as path.cpp but implementing both Direct and Indirect lighting by Next Event Estimation .
* path_nee_dof : Same version as path_nee with a depth of field effect.

Some of the results are shown below 

600 samples using path.cpp

![image](https://github.com/user-attachments/assets/63ba3533-b2e6-4240-984c-a0ea97c36534)

500 samples using path_nee.cpp

![image](https://github.com/user-attachments/assets/ca1b8000-9789-4da4-9d33-1f85ebf03981)

600 samples with path_nee_dof (10 samples of secondary rays and 0.1 aperture of the camera shutter, and at a focal distance of 5)

![image](https://github.com/user-attachments/assets/74d1acdd-d03a-47ee-a367-c91276a7a93c)
