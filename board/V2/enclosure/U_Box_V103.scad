
/*//////////////////////////////////////////////////////////////////
              -    FB Aka Heartman/Hearty 2016     -                   
              -   http://heartygfx.blogspot.com    -                  
              -       OpenScad Parametric Box      -                     
              -         CC BY-NC 3.0 License       -                      
////////////////////////////////////////////////////////////////////                                                                                                             
12/02/2016 - Fixed minor bug 
28/02/2016 - Added holes ventilation option                    
09/03/2016 - Added PCB feet support, fixed the shell artefact on export mode. 

*/////////////////////////// - Info - //////////////////////////////

// All coordinates are starting as integrated circuit pins.
// From the top view :

//   CoordD           <---       CoordC
//                                 ^
//                                 ^
//                                 ^
//   CoordA           --->       CoordB


////////////////////////////////////////////////////////////////////


////////// - Paramètres de la boite - Box parameters - /////////////

/* [Box dimensions] */
// - Longueur - Length  
  Length        = 110.5;       
// - Largeur - Width
  Width         = 81;                     
// - Hauteur - Height  
  Height        = 30;  
// - Epaisseur - Wall thickness  
  Thick         = 2;//[2:5]  
  
/* [Box options] */
// Pieds PCB - PCB feet (x4) 
  PCBFeet       = 1;// [0:No, 1:Yes]
// - Decorations to ventilation holes
  Vent          = 0;// [0:No, 1:Yes]
// - Decoration-Holes width (in mm)
  Vent_width    = 1.5;   
// - Text you want
  txt           = "HP85Disk";           
// - Font size  
  TxtSize       = 4;                 
// - Font  
  Police        ="Arial Black"; 
// - Diamètre Coin arrondi - Filet diameter  
  Filet         = 2;//[0.1:12] 
// - lissage de l'arrondi - Filet smoothness  
  Resolution    = 50;//[1:100] 
// - Tolérance - Tolerance (Panel/rails gap)
  m             = 0.9;
  
/* [PCB_Feet--TheBoard_Will_NotBeExported) ] */
//All dimensions are from the center foot axis
// - Coin bas gauche - Low left corner X position
PCBPosX         = -4;
// - Coin bas gauche - Low left corner Y position
PCBPosY         = -4.5
;
// - Longueur PCB - PCB Length
PCBLength       = 90;
// - Largeur PCB - PCB Width
PCBWidth        = 65;
// - Heuteur pied - Feet height
FootHeight      = 6.5;
// - Diamètre pied - Foot diameter
FootDia         = 8;
// - Diamètre trou - Hole diameter
FootHole        = 2.5;  
  

/* [STL element to export] */
//Coque haut - Top shell
  TShell        = 0;// [0:No, 1:Yes]
//Coque bas- Bottom shell
  BShell        = 1;// [0:No, 1:Yes]
//Panneau arrière - Back panel  
  BPanel        = 0;// [0:No, 1:Yes]
//Panneau avant - Front panel
  FPanel        = 1;// [0:No, 1:Yes]
//Texte façade - Front text
  Text          = 0;// [0:No, 1:Yes]


  
/* [Hidden] */
// - Couleur coque - Shell color  
Couleur1        = "Orange";       
// - Couleur panneaux - Panels color    
Couleur2        = "OrangeRed";    
// Thick X 2 - making decorations thicker if it is a vent to make sure they go through shell
Dec_Thick       = Vent ? Thick*2 : Thick; 
// - Depth decoration
Dec_size        = Vent ? Thick*2 : 0.8;

//////////////////// Oversize PCB limitation -Actually disabled - ////////////////////
//PCBL= PCBLength+PCBPosX>Length-(Thick*2+7) ? Length-(Thick*3+20+PCBPosX) : PCBLength;
//PCBW= PCBWidth+PCBPosY>Width-(Thick*2+10) ? Width-(Thick*2+12+PCBPosY) : PCBWidth;
PCBL=PCBLength;
PCBW=PCBWidth;
//echo (" PCBWidth = ",PCBW);



/////////// - Boitier générique bord arrondis - Generic Fileted box - //////////

module RoundBox($a=Length, $b=Width, $c=Height){// Cube bords arrondis
                    $fn=Resolution;            
                    translate([0,Filet,Filet]){  
                    minkowski (){                                              
                        cube ([$a-(Length/2),$b-(2*Filet),$c-(2*Filet)], center = false);
                        rotate([0,90,0]){    
                        cylinder(r=Filet,h=Length/2, center = false);
                            } 
                        }
                    }
                }// End of RoundBox Module

      
////////////////////////////////// - Module Coque/Shell - //////////////////////////////////         

module Coque(){//Coque - Shell  
    Thick = Thick*2;  
    difference(){    
        difference(){//sides decoration
            union(){    
                     difference() {//soustraction de la forme centrale - Substraction Fileted box
                      
                        difference(){//soustraction cube median - Median cube slicer
                            union() {//union               
                            difference(){//Coque    
                                RoundBox();
                                translate([Thick/2,Thick/2,Thick/2]){     
                                   //     RoundBox($a=Length-Thick, $b=Width-Thick, $c=Height-Thick);
                                       RoundBox($a=Length-Thick, $b=Width-Thick, $c=Height-Thick);
                                        }
                                        }//Fin diff Coque                            
                                difference(){//largeur Rails        
                                     translate([Thick+m,Thick/2,Thick/2]){// Rails
  //                                        RoundBox($a=Length-((2*Thick)+(2*m)), $b=Width-Thick, $c=Height-(Thick*2));
                                          RoundBox($a=Length-((2*Thick)+(2*m)), $b=Width-Thick, $c=Height-(Thick*2));
                                                          }//fin Rails
                                     translate([((Thick+m/2)*1.55),Thick/2,Thick/2+0.1]){ // +0.1 added to avoid the artefact
                                          RoundBox($a=Length-((Thick*3)+2*m), $b=Width-Thick, $c=Height-Thick);
                                                    }           
                                                }//Fin largeur Rails
                                    }//Fin union                                   
                               translate([-Thick,-Thick,Height/2]){// Cube à soustraire
                                    cube ([Length+100, Width+100, Height], center=false);
                                            }                                            
                                      }//fin soustraction cube median - End Median cube slicer
                               translate([-Thick/2,Thick,Thick]){// Forme de soustraction centrale 
                                    RoundBox($a=Length+Thick, $b=Width-Thick*2, $c=Height-Thick);       
                                    }                          
                                }                                          


                difference(){// Fixation box legs
                    union(){
                        translate([3*Thick +5,Thick,Height/2]){
                            rotate([90,0,0]){
                                    $fn=6;
                                    cylinder(d=16,Thick/2);
                                    }   
                            }
                            
                       translate([Length-((3*Thick)+5),Thick,Height/2]){
                            rotate([90,0,0]){
                                    $fn=6;
                                    cylinder(d=16,Thick/2);
                                    }   
                            }

                        }
                            translate([4,Thick+Filet,Height/2-57]){   
                             rotate([45,0,0]){
                                   cube([Length,40,40]);    
                                  }
                           }
                           translate([0,-(Thick*1.46),Height/2]){
                                cube([Length,Thick*2,10]);
                           }
                    } //Fin fixation box legs
            }

        union(){// outbox sides decorations
            //if(Thick==1){Thick=2;
            for(i=[0:Thick:Length/4]){

                // Ventilation holes part code submitted by Ettie - Thanks ;) 
                    translate([10+i,-Dec_Thick+Dec_size,1]){
                    cube([Vent_width,Dec_Thick,Height/4]);
                    }
                    translate([(Length-10) - i,-Dec_Thick+Dec_size,1]){
                    cube([Vent_width,Dec_Thick,Height/4]);
                    }
                    translate([(Length-10) - i,Width-Dec_size,1]){
                    cube([Vent_width,Dec_Thick,Height/4]);
                    }
                    translate([10+i,Width-Dec_size,1]){
                    cube([Vent_width,Dec_Thick,Height/4]);
                    }
  
                
                    }// fin de for
               // }
                }//fin union decoration
            }//fin difference decoration


            union(){ //sides holes
                $fn=50;
                translate([3*Thick+5,20,Height/2+4]){
                    rotate([90,0,0]){
                    cylinder(d=2,20);
                    }
                }
                translate([Length-((3*Thick)+5),20,Height/2+4]){
                    rotate([90,0,0]){
                    cylinder(d=2,20);
                    }
                }
                translate([3*Thick+5,Width+5,Height/2-4]){
                    rotate([90,0,0]){
                    cylinder(d=2,20);
                    }
                }
                translate([Length-((3*Thick)+5),Width+5,Height/2-4]){
                    rotate([90,0,0]){
                    cylinder(d=2,20);
                    }
                }
            }//fin de sides holes

        }//fin de difference holes
}// fin coque 

////////////////////////////// - Experiment - ///////////////////////////////////////////


///////////////////////////////// - Module Front/Back Panels - //////////////////////////
                            
module Panels(){//Panels
    color(Couleur2){
        translate([Thick+m,m/2,m/2]){
             difference(){
                  translate([0,Thick,Thick]){ 
                     RoundBox(Length,Width-((Thick*2)+m),Height-((Thick*2)+m));}
                  translate([Thick,-5,0]){
                     cube([Length,Width+10,Height]);}
                     }
                }
         }
}


/////////////////////// - Foot with base filet - /////////////////////////////
module foot(FootDia,FootHole,FootHeight){
    Filet=2;
     color("Green")       
   translate([0,0,Filet-1.5])
    difference(){
    
    difference(){
            //translate ([0,0,-Thick]){
                cylinder(d=FootDia+Filet,FootHeight-Thick, $fn=100);
                        //}
                    rotate_extrude($fn=100){
                            translate([(FootDia+Filet*2)/2,Filet,0]){
                                    minkowski(){
                                            square(10);
                                            circle(Filet, $fn=100);
                                       }
                                 }
                           }
                   }
            cylinder(d=FootHole,FootHeight+1, $fn=100);
               }          
}// Fin module foot
  
module Feet(){     
//////////////////// - PCB only visible in the preview mode - /////////////////////    
   translate([3*Thick+2,Thick+5,FootHeight+(Thick/2)-0.5]){
 //   color("Green")
 //   %square ([PCBL+10,PCBW+10]);
       translate([PCBL/2,PCBW/2,0.5]){ 
        color("Olive")
        %text("PCB", halign="center", valign="center", font="Arial black");
       }
    } // Fin PCB 
  
    
////////////////////////////// - 4 Feet - //////////////////////////////////////////     
    translate([3*Thick+7.5,Thick+10,Thick/2]){
        foot(FootDia,FootHole,FootHeight);
    }
    translate([(3*Thick)+PCBL+9.0,Thick+10,Thick/2]){
        foot(FootDia,FootHole,FootHeight);
        }
    translate([(3*Thick)+PCBL+9.0,(Thick)+PCBW+10,Thick/2]){
        foot(FootDia,FootHole,FootHeight);
        }        
    translate([3*Thick+7.5,(Thick)+PCBW+10,Thick/2]){
        foot(FootDia,FootHole,FootHeight);
    }   

} // Fin du module Feet
 

///////////////////////////////////// - Main - ///////////////////////////////////////

if(BPanel==1)
//Back Panel
translate ([-m/2,0,0]){
  difference(){
        Panels();   
    // HPIB CONNECTOR
    translate([2.25,12.5,7.5]){
        cube ([3,56,16.50], center = false);}
    }
}

if(FPanel==1)
//Front Panel
rotate([0,0,180]){
    translate([-Length-m/2,-Width,0]){  
     difference(){
     Panels();
     // POWER CONNECTOR
     translate([1,68.5-11.85,7.0]){
        cube ([5,9.5,12.0], center = false);}
     // USB CONNECTOR
     translate([1,2.0+12.0,6.5]){
        cube ([5,11,6.5], center = false);}
     // uSD slot
     translate([1,14,18]){
        cube ([5,15,3], center = false);}
 
    }

       }
   }

if(Text==1)
// Front text
color(Couleur1){     
     translate([Length-(Thick),Thick*4,(Height-(Thick*4+(TxtSize/2)))+2]){// x,y,z
          rotate([90,0,90]){
              linear_extrude(height = 0.25){
              text(txt, font = Police, size = TxtSize,  valign ="center", halign ="left");
                        }
                 }
         }
}


if(BShell==1)
// Coque bas - Bottom shell
color(Couleur1){ 
    difference(){
        Coque();
        // PCB Knockout
        translate([4.7,2.50,6.0]){       // Back, right
            cube ([2.5,2.5,3], center = false);}
        translate([4.7,75.75,6.0]){      // Back, Left
            cube ([2.5,2.5,3], center = false);}
       // PCB Knockout
        translate([103.3,2.50,6.0]){     // Front right
            cube ([2.5,2.5,3], center = false);}
        translate([103.3,75.75,6.0]){     // Front, Left
            cube ([2.5,2.5,3], center = false);}
        // uSD Card Knockout
        translate([88.5-28.5,78.00,7.5]){
            cube ([15.5,3.5,3.0], center = false);}

    }
}


if(TShell==1)
// Coque haut - Top Shell
color( Couleur1,1){
    translate([0,Width,Height+0.2]){
        rotate([0,180,180]){
                Coque();
                }
        }
}

if (PCBFeet==1)
// Feet
translate([PCBPosX,PCBPosY,0]){ 
Feet();
 }