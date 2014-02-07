width=25.400;
length=33.274;
height=0.5;
translate([-width/2, -length/2, 0]) 
difference() { 
	translate([0, 0, 0]) cube([width,length,height]); 
	translate([1.651, 18.542,0]) cylinder(100,1.25,1.25);
	translate([23.749, 18.542,0]) cylinder(100,1.25,1.25);
}
