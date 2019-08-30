#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "ConsoleWrapper.h"
#include "TcpServerWrapper.h"

Isolate* isolate_;
Isolate* GetIsolate() { return isolate_; }

void Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::exit(0);
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  //get script from args
  //printf( "opening script %s \n",argv[1]);
  FILE* file = fopen(argv[1],"r");
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);
  char* fileScript = new char[size + 1];
  fileScript[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&fileScript[i], 1, size - i, file);
    }
   fclose(file);
  
  // Create a new Isolate and make it the current one.
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
	  isolate_ = isolate;
    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);
    
    //create a template
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    //use print
    global->Set(
      v8::String::NewFromUtf8(isolate, "quit", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, Quit));
	   
    // Create a new context.
    Local<Context> context = Context::New(isolate,NULL,global);

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);
	
	//create js object
    Console* c = new Console();
    Local<Object> con = WrapObject(c);

    TcpServer* t = new TcpServer();
    Local<Object> tcp = WrapTcpServerObject(t);

	  context->Global()->Set(context,
		String::NewFromUtf8(isolate,"console",NewStringType::kNormal).ToLocalChecked(),
		con).FromJust();

	  context->Global()->Set(context,
		String::NewFromUtf8(isolate,"tcpServer",NewStringType::kNormal).ToLocalChecked(),
		tcp).FromJust();
     // Create a string containing the JavaScript source code.
    Local<String> source =
        String::NewFromUtf8(isolate, fileScript ,
                            NewStringType::kNormal).ToLocalChecked();


    // Compile the source code.
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();

    // Run the script to get the result.
    Local<Value> result = script->Run(context).ToLocalChecked();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(GetIsolate(),result);
    //printf("%s\n", *utf8);

  // The script compiled and ran correctly.  Now we fetch out the
  // Process function from the global object.
  //Local<String> process_name =
  //    String::NewFromUtf8(GetIsolate(), "createServer", NewStringType::kNormal)
  //        .ToLocalChecked();
  //Local<Value> process_val;
  // If there is no Process function, or if it is not a function,
  // bail out
  //if (context->Global()->Get(context, process_name).ToLocal(&process_val) &&
  //    process_val->IsFunction()) {
      // It is a function; cast it to a Function
  //    Local<Function> process_fun = Local<Function>::Cast(process_val);
  //    Handle<Value> args[1];
  //    args[0] = String::NewFromUtf8(GetIsolate(), "called from C++", NewStringType::kNormal)
  //        .ToLocalChecked();
  //    MaybeLocal<Value> js_result = process_fun->Call(context, context->Global(), 1, args);
  //}

}
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();

//  delete create_params.array_buffer_allocator;
  return 0;
}


