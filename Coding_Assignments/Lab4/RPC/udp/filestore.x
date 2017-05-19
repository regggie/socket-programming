
const SERVER_FILE_PATH = "filestore/";
const CLIENT_FILE_PATH = "clientstore/";
typedef string filename<255>;
typedef opaque filechunk<1024>;

struct request {
  filename name;
  int pos;
};

program FILE_STORE {
	version FILE_STORE_VER_1 {

	int GETFILESIZE(filename name)=1;
	filechunk GETFILEBYCHUNKS(request req)=2;

	}=1;
}=0x23453333;
