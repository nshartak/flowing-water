# flowing-water
An applicaiton that renders a flowing river and the terrain with the user being able to move the camera around also to
to create a ripple effect in the location clicked.
The application uses glfw for windowing and user interaction and glm for matrix operations.
![image](https://user-images.githubusercontent.com/81687265/221869146-c7926387-dbb9-496f-b019-7b8e3356a3cb.png)

To generate a ripple effect I slightly tuned and used the function $$sin(\sqrt{x^2+y^2})/\sqrt{x^2+y^2}$$

The ripple effect in wireframe mode: the blue dot is where the user clicked)
![image](https://user-images.githubusercontent.com/81687265/221873135-a9776e6a-b8d5-4d64-b39e-72e1317accf7.png)
![image](https://user-images.githubusercontent.com/81687265/221873560-6743411b-61ea-45e0-8bdc-04b0b6d045cd.png)
