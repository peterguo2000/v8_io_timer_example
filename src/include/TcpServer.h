#define SERVER_PORT 8000
#define MAXLINE 4096
enum IO_TYPE {IO_BLOCKING, IO_NON_BLOCKING, IO_MUTIPLEXING}; 
class TcpServer {
	public:
		static void start(IO_TYPE ioType);
};
