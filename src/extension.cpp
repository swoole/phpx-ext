#include "phpx.h"

BEGIN_EXTERN_C()
#include "extension_arginfo.h"
END_EXTERN_C()

#define EXT_NAME "test"
#define EXT_VERSION "1.0.0"

#include <iostream>

using namespace php;
using namespace std;

PHPX_FUNCTION(test_fn1) {
    for (int i = 0; i < args.count(); i++) {
        cout << args[i].type() << endl;
    }
    return 1234;
}

PHPX_FUNCTION(test_fn2) {
    for (int i = 0; i < args.count(); i++) {
        cout << args[i].type() << endl;
    }
    auto v1 = args[0];
    Array arr(v1);
    arr.set(1, "efg");

     php::echo("argc=%d\n", args.count());
     php::error(E_WARNING, "extension warning.");

    return arr;
}

PHPX_FUNCTION(test_throw_error) {
    throwException("RuntimeException", "phpx exception: test");
    return {};
}

PHPX_METHOD(MyClass, test) {
    cout << "MyClass::test" << endl;
    return 1234.56;
}

PHPX_METHOD(MyClass, pget) {
    String *str = _this.oGet<String>("resource", "ResourceString");
    cout << "[GET] ResourceString: " << str->length() << endl;
    return "hello xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

PHPX_METHOD(MyClass, pset) {
    String *str = new String("hello world");
    _this.oSet("resource", "ResourceString", str);
    cout << "[SET] ResourceString: " << str->length() << endl;
    return {};
}

PHPX_METHOD(MyClass, count) {
    return 100;
}

void string_dtor(zend_resource *res) {
    String *s = static_cast<String *>(res->ptr);
    delete s;
}

PHPX_EXTENSION() {
    auto *extension = new Extension(EXT_NAME, EXT_VERSION);

    extension->onStart = [extension]() noexcept {
        printf("onStart\n");
        extension->registerConstant("TEST_CONST", 10002);

        auto *c = new Class("MyClass");
        c->registerFunctions(class_MyClass_methods);
        c->addConstant("TEST_CONSTANT", 8888);
        c->addStaticProperty("testStaticProperty", "(static) hello world", ZEND_ACC_PUBLIC);
        c->implements(zend_ce_countable);
        c->alias("MyClassAlias");
        extension->registerClass(c);

        auto *e = new Class("MyException");
        e->extends(zend_ce_exception);
        extension->registerClass(e);

        auto *e2 = new Class("MyRuntimeException");
        e2->extends(e);
        extension->registerClass(e2);

        auto *i = new Interface("MyInterface");
        i->registerFunctions(class_MyInterface_methods);
        extension->registerInterface(i);
        extension->registerResource("ResourceString", string_dtor);
        const auto ce = i->ptr();
        printf("ce=%p\n", ce);
    };

    extension->onShutdown = [extension]() noexcept { cout << extension->name << "shutdown" << endl; };
    extension->onBeforeRequest = [extension]() noexcept { cout << extension->name << "beforeRequest" << endl; };
    extension->onAfterRequest = [extension]() noexcept { cout << extension->name << "afterRequest" << endl; };

    extension->addIniEntry("test.option", "9999", PHP_INI_ALL);
    extension->registerFunctions(ext_functions);
    extension->require("swoole");
    extension->info({EXT_NAME "support", "enabled"}, {
                        {"version", extension->version},
                    });

    return extension;
}
