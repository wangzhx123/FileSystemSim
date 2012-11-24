#include "fileSystem.h"
#include "namei.h"
#include <iostream>
using namespace std;

int main() {
	CFileSystem fs;
	char dir[] = "/abc/def";
	fs.mkdir2(dir);
	char dir2[] = "/abc/def2";
	fs.mkdir2(dir2);

	char file[] = "/abc/test.txt";
	int fd = fs.open(file);
	char buf[] = "if you can read me in the memory, then congratulations! it means that it finally works.";
	cout << "you are gonna to write " << sizeof(buf) << " bytes" << endl;
	fs.write(fd, buf, sizeof(buf));

	return 0;
}
