
TIMES = 5

algorithms = [ "System", "First Fit", "Best Fit", "Worst Fit", "Quick Fit" ]

# Compile everything!
system("make -B")

algorithms.each_index do | strategy |
 	print algorithms[strategy].ljust(10), "\t"

 	name = algorithms[strategy]
 	cmd = name.downcase.gsub(/ /, '');

	system("./memory_#{cmd} 64")
	system("./memory_#{cmd} 40960")
	system("./memory_#{cmd} 0")
	puts
 end