globals
	int j, k;
end

prototypes
	void tmp(int);
end

functions
	void tmp(int i) {
		printvar(i*k);
		return;
	};
end

begin
	j = 1;
	k = 3;
	tmp(j);
end
