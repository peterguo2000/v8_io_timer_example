#include "ConsoleWrapper.h"

extern Isolate* GetIsolate();

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void log(const FunctionCallbackInfo<Value>& args){
	v8::String::Utf8Value str(GetIsolate(), args[0]);    
	const char* cstr = ToCString(str);     
	printf("*** %s\n", cstr);
}

void error(const FunctionCallbackInfo<Value>& args){
	v8::String::Utf8Value str(GetIsolate(), args[0]);
	const char* cstr = ToCString(str);
	fprintf(stderr,"*** %s\n", cstr);
}

Local<Object> WrapObject(Console *c) {

	EscapableHandleScope handle_scope(GetIsolate());

	Local<ObjectTemplate> class_t;
	Local<ObjectTemplate> raw_t = ObjectTemplate::New(GetIsolate());
        raw_t->SetInternalFieldCount(1);
        raw_t->Set(
		v8::String::NewFromUtf8(GetIsolate(), "log", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(GetIsolate(), log));
	raw_t->Set(
		v8::String::NewFromUtf8(GetIsolate(), "error", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(GetIsolate(), error));
	class_t = Local<ObjectTemplate>::New(GetIsolate(),raw_t);
        //create instance
        Local<Object> result = class_t->NewInstance(GetIsolate()->GetCurrentContext()).ToLocalChecked();
	//create wrapper
        Local<External> ptr = External::New(GetIsolate(),c);
	result->SetInternalField(0,ptr);
	return handle_scope.Escape(result);
}
