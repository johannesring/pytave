function [testresult] = testfile(arg)
	 disp(arg)
	 disp(["Type info: " typeinfo(arg)])
	 testresult = arg;#int8([1]);
end
