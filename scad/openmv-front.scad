width=25.400;
length=27;
height=0.5;
translate([-width/2, -length/2, 0]) 
difference() { 
	//main cube
	translate([0, 0, 0]) cube([width,length,height]); 

	//mounting holes
	translate([1.545, 12.2,0])  cylinder(100,1.25,1.25);
	translate([23.876, 12.2,0]) cylinder(100,1.25,1.25);

	//sensor lens
	translate([13.10, 22.5, 0]) 	cylinder(100,4.25,4.25);
}
