function: int quad(double);
function: double square(double);

define: int quad(double d) {
	return int(square(d));
};

define: double square(double d) {
	return d*d;
};

printvar(square(2.0));
