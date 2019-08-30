#include "TcpServerWrapper.h"
#include <sys/time.h>

extern Isolate* GetIsolate();
v8::Persistent<v8::Function> r_call;
v8::Persistent<v8::Function> timeout_call;
int32_t msInterval;
struct timeval stopTime;

// Extracts a C string from a V8 Utf8Value.
extern char* ToCString(const v8::String::Utf8Value& value);

void startTimer() {
	//start timer
	struct timeval now;
	gettimeofday(&now, NULL);
	stopTime.tv_sec = now.tv_sec + msInterval/1000;
	stopTime.tv_usec = msInterval%1000 + now.tv_usec;
}

bool isTimeout() {
	struct timeval now;
	gettimeofday(&now, NULL);
	unsigned long nowValue = now.tv_sec * 1000000 + now.tv_usec;
	unsigned long stopValue = stopTime.tv_sec * 1000000 + stopTime.tv_usec;

	if(stopTime.tv_sec != 0 && nowValue >= stopValue ) {
		stopTime.tv_sec = 0;
		return true;
	} else {
		return false;
	}
}

void onTimeout(const char* msg) {
    if(!timeout_call.IsEmpty()) {
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(GetIsolate(), timeout_call);
        if (!func.IsEmpty()) {
            v8::Local<v8::Value> argv[1];
            argv[0] = String::NewFromUtf8(GetIsolate(), msg, NewStringType::kNormal)
                            .ToLocalChecked();
			Local<Context> context = GetIsolate()->GetCurrentContext();
            MaybeLocal<Value> js_result = func->Call(context, context->Global(), 1, argv);
        }
  	}
}

void runCallBack(const char* buf) {
    if(!r_call.IsEmpty()) {
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(GetIsolate(), r_call);
        if (!func.IsEmpty()) {
            v8::Local<v8::Value> argv[1];
            argv[0] = String::NewFromUtf8(GetIsolate(), buf, NewStringType::kNormal)
                            .ToLocalChecked();
			Local<Context> context = GetIsolate()->GetCurrentContext();
            MaybeLocal<Value> js_result = func->Call(context, context->Global(), 1, argv);
        }
  	}
}

void start(const FunctionCallbackInfo<Value>& args) {

	//set callback
	v8::Isolate* isolate = args.GetIsolate();
    if (args[1]->IsFunction()) {
        v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(args[1]);
        v8::Function * ptr = *func;
        r_call.Reset(isolate, func);
    }

	//start io 
	v8::String::Utf8Value str(GetIsolate(), args[0]);  
	const char* cstr = ToCString(str); 
	if(strcmp("IO_BLOCKING", cstr) == 0) {
		TcpServer::start(IO_BLOCKING);
	} else if (strcmp("IO_NON_BLOCKING", cstr) == 0) {
		TcpServer::start(IO_NON_BLOCKING);
	} else if (strcmp("IO_MUTIPLEXING", cstr) == 0) {
		TcpServer::start(IO_MUTIPLEXING);
	} else {
		fprintf(stderr,"Server mode %s is not support.\n", cstr);
	}	
}

void setTimeout(const FunctionCallbackInfo<Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
    if (args[1]->IsFunction()) {
        v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(args[1]);
        v8::Function * ptr = *func;
        timeout_call.Reset(isolate, func);
    } 

	msInterval = args[0]->Int32Value(GetIsolate()->GetCurrentContext()).FromJust();
}

Local<Object> WrapTcpServerObject(TcpServer *t) {

	EscapableHandleScope handle_scope(GetIsolate());

	Local<ObjectTemplate> class_t;
	Local<ObjectTemplate> raw_t = ObjectTemplate::New(GetIsolate());
    raw_t->SetInternalFieldCount(1);

    raw_t->Set(
	v8::String::NewFromUtf8(GetIsolate(), "start", v8::NewStringType::kNormal).ToLocalChecked(),
	v8::FunctionTemplate::New(GetIsolate(), start));

	raw_t->Set(
	v8::String::NewFromUtf8(GetIsolate(), "setTimeout", v8::NewStringType::kNormal).ToLocalChecked(),
	v8::FunctionTemplate::New(GetIsolate(), setTimeout));

	class_t = Local<ObjectTemplate>::New(GetIsolate(),raw_t);
	//create instance
    Local<Object> result = class_t->NewInstance(GetIsolate()->GetCurrentContext()).ToLocalChecked();
	//create wrapper
    Local<External> ptr = External::New(GetIsolate(),t);
	result->SetInternalField(0,ptr);
	return handle_scope.Escape(result);
}
