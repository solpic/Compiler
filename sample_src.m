globals
end

structs
	pt{
		int x, y;
		int z;
	};
	ln{
		pt a, b;
	};
end

prototypes
	void print_pt(pt);
	pt add_pt(pt, pt);
	void print_ln(ln*);
	ln mk_ln(int, int, int, int, int, int);
	
	int* malloc(int) hook;
	void delete(int*) hook;
	void print_ar(int*, int);
	void assign_ar(int*, int);
end

functions
	void assign_ar(int *a, int size) {
		int i = 0;
		while(i<size) {
			a[i] = i*i;
			i = i + 1;
		}
		return;
	};
	void print_ar(int *a, int size) {
		int i = 0;
		while(i<size) {
			printvar(i);
			print(": ");
			printvar(a[i]);
			print("\n");
			i = i + 1;
		}
		return;
	};
	pt add_pt(pt a, pt b) {
		a.x = a.x + b.x;
		a.y = a.y + b.y;
		a.z = a.z + b.z;
		return a;
	};
	void print_pt(pt i) {
		printvar(i.x);
		print(", ");
		printvar(i.y);
		print(", ");
		printvar(i.z);
		return;
	};
	void print_ln(ln *l) {
		print("Line: ");
		print_pt((*l).a);
		print(" -> ");
		print_pt((*l).b);
		print("\n");
		return;
	};
	ln mk_ln(int a, int b, int c, int d, int e, int f) {
		pt x;
		x.x = a;
		x.y = b;
		x.z = c;
		pt y;
		y.x = d;
		y.y = e;
		y.z = f;
		ln z;
		z.a = x;
		z.b = y;
		return z;
	};
end
/*
Struct ops
q.x = 1;
q->x = 2;
*/

begin
	int *a = int*(malloc(10*sizeof(int)));
	assign_ar(a, 10);
	print_ar(a, 10);
	delete(a);
end
