globals
	int j = 1, k = 5;
end

prototypes
	void tmp(int);
	void tmp2(int*);
end

functions
	void tmp(int i) {
		int q;
		printvar(i*k);
		return;
	};
	void tmp2(int* q) {
		*q = 10;
		return;
	};
end

begin
	tmp(j);
	printvar(j);
	tmp2(&j);
	printvar(j);
end
