The enclosure CAD file was downloaded from Thingiverse with the copyright shown below the dotted line. 
This is an OpenSCAD file. 
OpenSCAD can be downloaded from www.openscad.org.
Modifications were made to the original file to fit with the HP85Disk pcb V2.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

                   .:                     :,                                          
,:::::::: ::`      :::                   :::                                          
,:::::::: ::`      :::                   :::                                          
.,,:::,,, ::`.:,   ... .. .:,     .:. ..`... ..`   ..   .:,    .. ::  .::,     .:,`   
   ,::    :::::::  ::, :::::::  `:::::::.,:: :::  ::: .::::::  ::::: ::::::  .::::::  
   ,::    :::::::: ::, :::::::: ::::::::.,:: :::  ::: :::,:::, ::::: ::::::, :::::::: 
   ,::    :::  ::: ::, :::  :::`::.  :::.,::  ::,`::`:::   ::: :::  `::,`   :::   ::: 
   ,::    ::.  ::: ::, ::`  :::.::    ::.,::  :::::: ::::::::: ::`   :::::: ::::::::: 
   ,::    ::.  ::: ::, ::`  :::.::    ::.,::  .::::: ::::::::: ::`    ::::::::::::::: 
   ,::    ::.  ::: ::, ::`  ::: ::: `:::.,::   ::::  :::`  ,,, ::`  .::  :::.::.  ,,, 
   ,::    ::.  ::: ::, ::`  ::: ::::::::.,::   ::::   :::::::` ::`   ::::::: :::::::. 
   ,::    ::.  ::: ::, ::`  :::  :::::::`,::    ::.    :::::`  ::`   ::::::   :::::.  
                                ::,  ,::                               ``             
                                ::::::::                                              
                                 ::::::                                               
                                  `,,`


https://www.thingiverse.com/thing:1355018
The Ultimate Parametric Box by Heartman is licensed under the Creative Commons - Attribution - Non-Commercial license.
http://creativecommons.org/licenses/by-nc/3.0/

# Summary

NOTE: Check <a href="http://www.thingiverse.com/thing:1264391">this release</a> for an up-to-date version, including the Panel maker.

<br>
<br>
<img src="https://lh3.googleusercontent.com/PR-XvVAerEO8SpnojSW1q398_mLCCEbckJ1fvTelD_dwK95ricj2LsbhRBMlo5007Jrt0M6MunCTvvy5EHpvnQrA0IuMe3KioxrO8l_ge14aLKWRqOUF1rDkKoX-mqLacWRFifPlqd17x6uI2hcsopPaz0_psorhxP0KO9QicIL_x_yLQokEvcCLDJI7KaoxyWjxq1EIg5lv1NzGQpu2SF12k9hAgxGmqGvDSRrYPUJoveyxjLTK9M1IH7GRFmlkLYUuUIRqkt_d3PU4vRkA2_17-mlP1FWLzlRll4OLzhC5AIqNOQMCfVpUZvB9NU2579plcgupKSJE5bzefcHXuHPC7fO5fPLzqzZHuwz8uAdOkWWxXwrJ7HkVR2PXO3K9zlKhX4FS0jRq1Q9fAZWLxEp96iVSGEcVsEKA6q5o6erzLsS9M3Cfw2xiYrVu1gDXK5He00tWHahOkpbulZTEKLGH-9qozXtWafDRGJgwsqRWURfl1DA2rg-_X3k6zfMoZYP6pPEUgqQFh5uKfaBvFlRnx8_dhDCq1aE1aACl8vILFxJfkQOmVNPaWZGfT9OHZatXVhBVLFUGhGs5gAizL6VsGaeu3lhEogu3cy8=s100-no"> this project...
<br>
<br>

As any electronic hobbyist, I always need some enclosures to protect my circuits, and of course make them more user friendly.
More over, with the news mini/micro/nano boards, like Arduino, Raspberry/Banana/Orange pies and so on, enclosures are more needed than never before.

Since a moment I'm thinking about taking a look at OpenScad, just to see what can we do with, and learn how it works.
Plus
I love the Thingiverse Customizer, very usefull for those who don't know how to modelise in 3D, or basically, want to have something fast and easy.

So, I started this project....

It was a good starting way to learn, for me.

<font size="1">Note: I'm French, I did my best to make this presentation without spelling errors, please, correct me if something burned your eyes.</font>



# How I Designed This

## Let's start from the beginning.

Before to find a good design, what can we do with OpenScad.

Usually, in almost all programming language, we need to begin by something. 
The famous "Hello World"

In OpenScad, that will be something like:

<strong><font color="orange">cube([20,10,10]);</font></strong>
Means, draw a cube in the ([X, Y, Z]) dimensions (<i>values are in mm that will be the default unit for printing</i>)
<br>
<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold.jpg">
<font size="1"><i>For all images, right click on the pic, and choose "view image" to watch it in full size.</i></font>
<br><br><br>



## Press F5 and...

Compilation and rendering

<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold_Result.jpg">
<br><br><br>

## Let's go further.

I want now to draw another box and place it inside the first one.
I'm using the <strong><font color="orange">translate</font></strong> command to move this second box in <strong><font color="orange">([X,Y,Z])</font></strong> axis values.
In the example, 1 mm all around, to be centered inside the first one.
<i> - note: the <strong><font color="orange">%</font></strong> is just used to make the first box transparent for the demonstration or debugging. -</i> 
<br>
<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold_3.jpg">
<br><br><br>

## Booleans

Now, I would like to substract the second box from the first one.
I will use the boolean operation, <strong><font color="orange">difference</font></strong>, which will substract the second translated object, between the bracket, from the first one.
<br>
<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold_4.jpg">
<br><br><br>

## Variables

Here is the interesting part.
After trying what I want, and how I want it.
I replace the values by the named variables, to manage and simplify any size modifications. 
<br>
<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold_5.jpg">
<br><br><br>

## Experimenting

I had a part in my code for experimenting some read stuff on the documentation,
like the <strong><font color="orange">color</font></strong> or <strong><font color="orange">Minkowski</font></strong> functions, like debugging characters... <a href="http://www.openscad.org/cheatsheet/index.html">this page</a> was always open.
<br>
<img src="http://heartygfx.free.fr/Thingiverse/OpenScad-HelloWold_6.jpg">
<br>
This is the starting way I followed to design the Ultimate Parametric Box.
<br>

## Customizer

The Thingiverse Customizer application is able to link easily the variables needed, and make them editable by anyone, online.
To use the customizer for this project, I checked the well done <a href="http://customizer.makerbot.com/docs">documentation</a>.

Also,
The <a href="http://www.thingiverse.com/thingiversity">Thingiverstity</a>, more precisely <a href="http://www.thingiverse.com/jumpstart/openscad">this video</a> helped me a lot on how to begin with OpenScad.
And of course the Openscad online documentation was always under my eyes.
<br>


# Project: The Ultimate Parametric Box

## Objectives

- Design a housing mainly intended for the use of DIY electronic assemblies, control panels, or enclosures for any mini/micro/nano boards like Arduino, Rasberry pi and so on.

- The box design need to think about:
        a - aesthetic
        b - ergonomics
        c - structural reliability
        d - 3D printing constraints (<i><font size="2">Effect of layer thickness and printing orientation on mechanical properties</font><i>)

- Learn the base of programming language, such as C language-like syntax, Variables, loops, conditions, and finally have the algorithmic foundations in a visual way. 
<br>
- Develop a fully configurable box enclosure, easy access for the end user with the Makerbot customizer online interface.

<br>
<img src="http://heartygfx.free.fr/Thingiverse/Box_02.jpg">
<br><br><br>


## Audiences

- Any level. 
- Can be a collaborative project

## Preparation

* Computer with OpenScad installed, 

* Internet access is also recommended to help the students to find inspiration about the design, and/or the mechanical constraints, also the Openscad documentation.

* A 3D printer.
<br>




## Step 1

Define the what kind of box we want.
How to close the box: screws, sliders, lock clips
<br>
<img src="http://heartygfx.free.fr/Thingiverse/Which_box.jpg">
<br>


## Step 2

Translate the box defined in the step one to Openscad primitives shapes.

## Step 3

Finalizing the Step two code with variables in order to have a parametric box.

# Result

<br>
After thinking a lot to find a good concept, easy to print, easy to use, easy to build for anyone.

<br>
<img src="http://heartygfx.free.fr/Thingiverse/Thinking.jpg">
<br><br><br>


here is the result.
<br><br>
<img src="http://heartygfx.free.fr/Thingiverse/Box_03.jpg">
<br><br><br>

<img src="http://heartygfx.free.fr/BlogImg/Thingiverse/Ultimate_Box.gif">
<font size="1"><i>right click on the animated pic, and choose "view image" to watch it in full size</i></font>
<br><br><br>

<img src="http://heartygfx.free.fr/Thingiverse/Box1.jpg">
<br><br><br>
<img src="http://heartygfx.free.fr/Thingiverse/Box2.jpg">
<br><br><br>

* This, could be an awesome classroom project (!)
* The tools used to design and learn are free (!!)

What else...

;)