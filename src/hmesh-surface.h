/* This header files  defines functions corresponding ..
.. to half-edge mesh related to discrete surface      ..
.. representation of interface. Have tried to expand..
.. for(direction) loops wherever possible.*/

//centroid of halfface
coord
hface_centroid(Hedge * h){
	Frontpoint * v[3] = hface_vertices(h);
	double * x[3] = {v[0]->x, v[1]->x, v[2]->x};
	double c[3] = {
	  (x[0][0] + x[1][0] + x[2][0])/3.,
	  (x[0][1] + x[1][1] + x[2][1])/3.,
	  (x[0][2] + x[1][2] + x[2][2])/3.
	};
	return ((coord) {c[0],c[1],c[2]});
}

//area of halface
double
hface_area(Hedge * h){
	Frontpoint * p[3] = hface_vertices(h);
	double * x[3] = {p[0]->x, p[1]->x, p[2]->x};
	double u[3] = {
	  x[1][0]-x[0][0], 
	  x[1][1]-x[0][1],
	  x[1][2]-x[0][2]
	};
	double v[3] = {
	  x[2][0]-x[1][0], 
	  x[2][1]-x[1][1], 
	  x[2][2]-x[1][2]
	};
	double n[3] = {
	  u[1]*v[2]-u[2]*v[1],
	  u[2]*v[0]-u[0]*v[2],
	  u[0]*v[1]-u[1]*v[0]
	};

	return (sqrt(n[0]*n[0] + n[1]*n[1] +n[2]*n[2]));
}

//normal of halface
coord
hface_normal(Hedge * h, int area){
	Frontpoint * p[3] = hface_vertices(h);
	double * x[3] = {p[0]->x, p[1]->x, p[2]->x};
	double u[3] = {
	  x[1][0]-x[0][0], 
	  x[1][1]-x[0][1],
	  x[1][2]-x[0][2]
	};
	double v[3] = {
	  x[2][0]-x[1][0], 
	  x[2][1]-x[1][1], 
	  x[2][2]-x[1][2]
	};
	double n[3] = {
	  u[1]*v[2]-u[2]*v[1],
	  u[2]*v[0]-u[0]*v[2],
	  u[0]*v[1]-u[1]*v[0]
	};

	if(area)
		return ((coord) {n[0], n[1], n[2]});

	double norm = sqrt(n[0]*n[0] + n[1]*n[1] +n[2]*n[2]);
	return ((coord) {n[0]/norm, n[1]/norm, n[2]/norm});
}

//return the centroid of a halfedge
coord 
halfedge_ (void * obj){
	Hedge * h = (Hedge *) obj;
	Frontpoint * v[2] = hedge_vertices(h);
	double * x[2] = {v[0]->x, v[1]->x};
	double c[3] = {
		 (x[0][0] + x[1][0])/2.,
		 (x[0][1] + x[1][1])/2.,
		 (x[0][2] + x[1][2])/2.
	};
	return ((coord) {c[0], c[1], c[2]});
}

//return the length of halfedge
double
hedge_length (Hedge * h){
	Frontpoint * v[2] = hedge_vertices(h);
	double * p = v[0]->x, * q = v[1]->x;
	return (distance (p,q));
}

//Set normal at each pivots.
void
hmesh_normal (Front * fr){
	foreach_frontpoint(fr){
		//set normal and area as zero;
		double * n = frontpoint->T;	
		n[0] = 0.; n[1] = 0.; n[2] = 0.;
	}
	
	//vertex normal is the normalized vector of ..
	//.. area average of face normals ..
	//.. of the faces that share the vertex (Called as "fan");
	foreach_hedge(fr) {
		Frontpoint * v = __h->v;
		double * n = v->T;
		//area normal of the face
		coord nf = hface_normal(__h, 1);
		n[0] += nf.x; n[1] += nf.y; n[2] += nf.z;
	}

	// averaging
	foreach_frontpoint(fr){
		double * n = frontpoint->T;
		double norm = 
		  max(1E-13, sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]));
		n[0] /= norm; n[1] /= norm; n[2] /= norm;
	}
}

//Quality: Asp ratio of triangle. 
//Q \in [0,1]. 1 for equilateral tri
double
hface_quality(Hedge * h){
	double l[3] = { hedge_length(h),
	  hedge_length(hnext(h)), hedge_length(hnext(hnext(h)))};
	double S = hface_area(h);
	return (4. * sqrt(3) * S /(l[0]*l[0] + l[1]*l[1] + l[2]*l[2]));
	//Another formulation (not same) is 
	// Q = 12 sqrt(3) A / sq(P) , A:area, P is perimeter = a+b+c
}

double
htriangle_quality(double * x[]){
	double u[3] = {
	  x[1][0]-x[0][0], 
	  x[1][1]-x[0][1],
	  x[1][2]-x[0][2]
	};
	double v[3] = {
	  x[2][0]-x[1][0], 
	  x[2][1]-x[1][1], 
	  x[2][2]-x[1][2]
	};
	double w[3] = {
	  x[0][0]-x[2][0], 
	  x[0][1]-x[2][1], 
	  x[0][2]-x[2][2]
	};
	double n[3] = {
	  u[1]*v[2]-u[2]*v[1],
	  u[2]*v[0]-u[0]*v[2],
	  u[0]*v[1]-u[1]*v[0]
	};

	double A = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	double L2 =  u[0]*u[0] + u[1]*u[1] + u[2]*u[2] + 
	    v[0]*v[0] + v[1]*v[1] + w[2]*w[2] +
	    w[0]*w[0] + w[1]*w[1] + w[2]*w[2];

	return (4*sqrt(3)*A/L2);

}

//Find the coordinate on the mapping of an edge.
//Mapping: mapping points from C0 front ..
//.. to the approximate interface which is C1 or C2 surf
coord
hedge_interpolate(Hedge * h, double s){
	//Assume s \in [0,1]; 
	//Assume hmesh_normal() is set.

	Frontpoint * v[2] = hedge_vertices(h);
	double * x[2] = {v[0]->x, v[1]->x};
	double * n[2] = {v[0]->T, v[1]->T};
	
	//e = edge vector
	double t0[3] = {
	  x[1][0] - x[0][0],
	  x[1][1] - x[0][1],
	  x[1][2] - x[0][2]
	}, t1[3];
	
	memcpy(t1, t0, 3*sizeof(double));
	double * t[2] = {t0, t1};
	double l = max(1E-13, sqrt(t0[0]*t0[0] +
	  t0[1]*t0[1] + t0[2]*t0[2]));

	//tangent (in the tangent plane) at each vertex [iv].
	//constraint: 
	//..  normal[iv], tangent[iv], edge vector are in same plane.
	//Requirement (general requirement of surface mesh) ..
	// .. || n[iv] x e || > 0
	for(int iv = 0; iv < 2; ++iv) {
		double p = t[iv][0]*n[iv][0] + 
		  t[iv][1]*n[iv][1] + t[iv][2]*n[iv][2];
		//remove projection of normal to get the tangent at vertex
		t[iv][0] -= p*n[iv][0];
		t[iv][1] -= p*n[iv][1];
		t[iv][2] -= p*n[iv][2];
		//info: t can also derived as t[iv] = -((n[iv] x e) x e)
		//normalize and then multiply with edge length
		double dt = l/max(1E-13, sqrt(t[iv][0]*t[iv][0] + 
		  t[iv][1]*t[iv][1] + t[iv][2]*t[iv][2]));
		t[iv][0] *= dt; t[iv][1] *= dt; t[iv][2] *= dt; 
	}

	double _x[3];

	//fixme: expand this also	
	for(int d=0; d<3; ++d)
	{
		//Hermite interpolation btw two pivots (given their ..
		//..tgts in their repective tangent plane)
		_x[d] = x[0][d] 
		   + s * t[0][d] 
		   + sq(s) * (-3*(x[0][d] - x[1][d]) - 2*t[0][d] - t[1][d])
		   + cube(s) * (2*(x[0][d] - x[1][d]) + t[0][d] + t[1][d]); 
	}

	return ((coord) {_x[0], _x[1], _x[2]});		
		
}

coord 
hface_interpolate(Hedge * f, double u, double v) {
	//assume u,v, u+v \in [0,1]
	//assume hmesh_normal() is already called

	assert(false);
	//can see an implementation of 3d version of ..
	//..hermite interpoln as in hedge_interpolate() here ..
	//https://www.mdpi.com/2075-1680/12/4/370
}
