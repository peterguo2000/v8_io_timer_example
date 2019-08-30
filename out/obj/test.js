var x = 1;
var y = 2;
var z = x + y;
console.log(z);

tcpServer.setTimeout(10000, function onTimeout(arg) {
  console.log(`arg was => ${arg}`);
});

//IO_BLOCKING, IO_NON_BLOCKING, IO_MUTIPLEXING
tcpServer.start("IO_MUTIPLEXING", function test_function(fromC) {
        console.log("Message from client:  " + fromC);
});
