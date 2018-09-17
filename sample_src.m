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
end

functions
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
	ln a = mk_ln(1, 2, 3, 4, 5, 6);
	ln *b = &a;
	print_ln(b);
	*b = mk_ln(5, 6, 7, 8, 9, 10);
	print_ln(b);
	print_ln(&a);
end
