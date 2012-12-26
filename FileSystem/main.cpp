#include "fileSystem.h"
#include "namei.h"
#include <iostream>
using namespace std;

int main() {
	CFileSystem fs;

	/* test for mkdir2()
	for (int i = 0; i < 1024/16+5; i++) {
		char dir[14];
		sprintf(dir, "/abc/def%d", i);
		fs.mkdir2(dir);
	}*/
	
	char dir[] = "/abc/def";
	fs.mkdir2(dir);
	char dir2[] = "/abc/def2";
	fs.mkdir2(dir2);
	

	// test for namei2.cpp
	char file[] = "/abc/test.txt";
	int fd = fs.open(file);
	char buf[1024*2+512];
	for (int i = 0; i < 1024*2 + 512; i++) {
		buf[i] = i % 128;
	}
	cout << "you are gonna to write " << sizeof(buf) << " bytes" << endl;
	fs.write(fd, buf, sizeof(buf));
	char buf2[1024*3];
	memset(buf2, 0, sizeof(buf2));
	fs.read(fd, buf2, 1024*2);
	for (int i = 0; i < 1024*2; i++) {
		cout << buf2[i] << endl;
	}

	cout << "TEST FINISHED!" << endl;
	return 0;
}
