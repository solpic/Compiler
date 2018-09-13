globals
	int z = 1;
end

prototypes
	void tmp();
end

functions
	void tmp() {
		int p, l, r;
		return;
	};
end

begin
	int j = 0;
	while(j<10) {
		print("Hello\n");
		j = j + 1;
	}
	
	char k = 'd';
	printvar(k);
end
