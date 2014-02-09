width=25.400;
length=33.274;
height=0.5;
translate([-width/2, -length/2, 0]) 
difference() { 
	translate([0, 0, 0]) cube([width,length,height]); 
	translate([1.524, 18.415,0]) cylinder(100,1.20,1.20);
	translate([23.876, 18.415,0]) cylinder(100,1.20,1.20);
	translate([0, 10.8, 0]) cube([4.3,6.350,height]); 
}
