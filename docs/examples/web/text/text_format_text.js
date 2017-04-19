// The Module object: Our interface to the outside world. We import
// and export values on it, and do the work to get that through
// closure compiler if necessary. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(Module) { ..generated code.. }
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to do an eval in order to handle the closure compiler
// case, where this code here is minified but Module was defined
// elsewhere (e.g. case 4 above). We also need to check if Module
// already exists (e.g. case 3 above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module;
if (!Module) Module = (typeof Module !== 'undefined' ? Module : null) || {};

// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = {};
for (var key in Module) {
  if (Module.hasOwnProperty(key)) {
    moduleOverrides[key] = Module[key];
  }
}

// The environment setup code below is customized to use Module.
// *** Environment setup code ***
var ENVIRONMENT_IS_WEB = false;
var ENVIRONMENT_IS_WORKER = false;
var ENVIRONMENT_IS_NODE = false;
var ENVIRONMENT_IS_SHELL = false;

// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() thread proxied to worker. (with Emscripten -s PROXY_TO_WORKER=1) (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)

if (Module['ENVIRONMENT']) {
  if (Module['ENVIRONMENT'] === 'WEB') {
    ENVIRONMENT_IS_WEB = true;
  } else if (Module['ENVIRONMENT'] === 'WORKER') {
    ENVIRONMENT_IS_WORKER = true;
  } else if (Module['ENVIRONMENT'] === 'NODE') {
    ENVIRONMENT_IS_NODE = true;
  } else if (Module['ENVIRONMENT'] === 'SHELL') {
    ENVIRONMENT_IS_SHELL = true;
  } else {
    throw new Error('The provided Module[\'ENVIRONMENT\'] value is not valid. It must be one of: WEB|WORKER|NODE|SHELL.');
  }
} else {
  ENVIRONMENT_IS_WEB = typeof window === 'object';
  ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
  ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function' && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER;
  ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
}


if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  if (!Module['print']) Module['print'] = console.log;
  if (!Module['printErr']) Module['printErr'] = console.warn;

  var nodeFS;
  var nodePath;

  Module['read'] = function read(filename, binary) {
    if (!nodeFS) nodeFS = require('fs');
    if (!nodePath) nodePath = require('path');
    filename = nodePath['normalize'](filename);
    var ret = nodeFS['readFileSync'](filename);
    return binary ? ret : ret.toString();
  };

  Module['readBinary'] = function readBinary(filename) {
    var ret = Module['read'](filename, true);
    if (!ret.buffer) {
      ret = new Uint8Array(ret);
    }
    assert(ret.buffer);
    return ret;
  };

  Module['load'] = function load(f) {
    globalEval(read(f));
  };

  if (!Module['thisProgram']) {
    if (process['argv'].length > 1) {
      Module['thisProgram'] = process['argv'][1].replace(/\\/g, '/');
    } else {
      Module['thisProgram'] = 'unknown-program';
    }
  }

  Module['arguments'] = process['argv'].slice(2);

  if (typeof module !== 'undefined') {
    module['exports'] = Module;
  }

  process['on']('uncaughtException', function(ex) {
    // suppress ExitStatus exceptions from showing an error
    if (!(ex instanceof ExitStatus)) {
      throw ex;
    }
  });

  Module['inspect'] = function () { return '[Emscripten Module object]'; };
}
else if (ENVIRONMENT_IS_SHELL) {
  if (!Module['print']) Module['print'] = print;
  if (typeof printErr != 'undefined') Module['printErr'] = printErr; // not present in v8 or older sm

  if (typeof read != 'undefined') {
    Module['read'] = read;
  } else {
    Module['read'] = function read() { throw 'no read() available' };
  }

  Module['readBinary'] = function readBinary(f) {
    if (typeof readbuffer === 'function') {
      return new Uint8Array(readbuffer(f));
    }
    var data = read(f, 'binary');
    assert(typeof data === 'object');
    return data;
  };

  if (typeof scriptArgs != 'undefined') {
    Module['arguments'] = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }

  if (typeof quit === 'function') {
    Module['quit'] = function(status, toThrow) {
      quit(status);
    }
  }

}
else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  Module['read'] = function read(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (ENVIRONMENT_IS_WORKER) {
    Module['readBinary'] = function read(url) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
      return xhr.response;
    };
  }

  Module['readAsync'] = function readAsync(url, onload, onerror) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onload = function xhr_onload() {
      if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
        onload(xhr.response);
      } else {
        onerror();
      }
    };
    xhr.onerror = onerror;
    xhr.send(null);
  };

  if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }

  if (typeof console !== 'undefined') {
    if (!Module['print']) Module['print'] = function print(x) {
      console.log(x);
    };
    if (!Module['printErr']) Module['printErr'] = function printErr(x) {
      console.warn(x);
    };
  } else {
    // Probably a worker, and without console.log. We can do very little here...
    var TRY_USE_DUMP = false;
    if (!Module['print']) Module['print'] = (TRY_USE_DUMP && (typeof(dump) !== "undefined") ? (function(x) {
      dump(x);
    }) : (function(x) {
      // self.postMessage(x); // enable this if you want stdout to be sent as messages
    }));
  }

  if (ENVIRONMENT_IS_WORKER) {
    Module['load'] = importScripts;
  }

  if (typeof Module['setWindowTitle'] === 'undefined') {
    Module['setWindowTitle'] = function(title) { document.title = title };
  }
}
else {
  // Unreachable because SHELL is dependant on the others
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}
if (!Module['load'] && Module['read']) {
  Module['load'] = function load(f) {
    globalEval(Module['read'](f));
  };
}
if (!Module['print']) {
  Module['print'] = function(){};
}
if (!Module['printErr']) {
  Module['printErr'] = Module['print'];
}
if (!Module['arguments']) {
  Module['arguments'] = [];
}
if (!Module['thisProgram']) {
  Module['thisProgram'] = './this.program';
}
if (!Module['quit']) {
  Module['quit'] = function(status, toThrow) {
    throw toThrow;
  }
}

// *** Environment setup code ***

// Closure helpers
Module.print = Module['print'];
Module.printErr = Module['printErr'];

// Callbacks
Module['preRun'] = [];
Module['postRun'] = [];

// Merge back in the overrides
for (var key in moduleOverrides) {
  if (moduleOverrides.hasOwnProperty(key)) {
    Module[key] = moduleOverrides[key];
  }
}
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used e.g. in memoryInitializerRequest, which is a large typed array.
moduleOverrides = undefined;



// {{PREAMBLE_ADDITIONS}}

// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

//========================================
// Runtime code shared with compiler
//========================================

var Runtime = {
  setTempRet0: function (value) {
    tempRet0 = value;
    return value;
  },
  getTempRet0: function () {
    return tempRet0;
  },
  stackSave: function () {
    return STACKTOP;
  },
  stackRestore: function (stackTop) {
    STACKTOP = stackTop;
  },
  getNativeTypeSize: function (type) {
    switch (type) {
      case 'i1': case 'i8': return 1;
      case 'i16': return 2;
      case 'i32': return 4;
      case 'i64': return 8;
      case 'float': return 4;
      case 'double': return 8;
      default: {
        if (type[type.length-1] === '*') {
          return Runtime.QUANTUM_SIZE; // A pointer
        } else if (type[0] === 'i') {
          var bits = parseInt(type.substr(1));
          assert(bits % 8 === 0);
          return bits/8;
        } else {
          return 0;
        }
      }
    }
  },
  getNativeFieldSize: function (type) {
    return Math.max(Runtime.getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },
  STACK_ALIGN: 16,
  prepVararg: function (ptr, type) {
    if (type === 'double' || type === 'i64') {
      // move so the load is aligned
      if (ptr & 7) {
        assert((ptr & 7) === 4);
        ptr += 4;
      }
    } else {
      assert((ptr & 3) === 0);
    }
    return ptr;
  },
  getAlignSize: function (type, size, vararg) {
    // we align i64s and doubles on 64-bit boundaries, unlike x86
    if (!vararg && (type == 'i64' || type == 'double')) return 8;
    if (!type) return Math.min(size, 8); // align structures internally to 64 bits
    return Math.min(size || (type ? Runtime.getNativeFieldSize(type) : 0), Runtime.QUANTUM_SIZE);
  },
  dynCall: function (sig, ptr, args) {
    if (args && args.length) {
      assert(args.length == sig.length-1);
      assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
      return Module['dynCall_' + sig].apply(null, [ptr].concat(args));
    } else {
      assert(sig.length == 1);
      assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
      return Module['dynCall_' + sig].call(null, ptr);
    }
  },
  functionPointers: [],
  addFunction: function (func) {
    for (var i = 0; i < Runtime.functionPointers.length; i++) {
      if (!Runtime.functionPointers[i]) {
        Runtime.functionPointers[i] = func;
        return 2*(1 + i);
      }
    }
    throw 'Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.';
  },
  removeFunction: function (index) {
    Runtime.functionPointers[(index-2)/2] = null;
  },
  warnOnce: function (text) {
    if (!Runtime.warnOnce.shown) Runtime.warnOnce.shown = {};
    if (!Runtime.warnOnce.shown[text]) {
      Runtime.warnOnce.shown[text] = 1;
      Module.printErr(text);
    }
  },
  funcWrappers: {},
  getFuncWrapper: function (func, sig) {
    assert(sig);
    if (!Runtime.funcWrappers[sig]) {
      Runtime.funcWrappers[sig] = {};
    }
    var sigCache = Runtime.funcWrappers[sig];
    if (!sigCache[func]) {
      // optimize away arguments usage in common cases
      if (sig.length === 1) {
        sigCache[func] = function dynCall_wrapper() {
          return Runtime.dynCall(sig, func);
        };
      } else if (sig.length === 2) {
        sigCache[func] = function dynCall_wrapper(arg) {
          return Runtime.dynCall(sig, func, [arg]);
        };
      } else {
        // general case
        sigCache[func] = function dynCall_wrapper() {
          return Runtime.dynCall(sig, func, Array.prototype.slice.call(arguments));
        };
      }
    }
    return sigCache[func];
  },
  getCompilerSetting: function (name) {
    throw 'You must build with -s RETAIN_COMPILER_SETTINGS=1 for Runtime.getCompilerSetting or emscripten_get_compiler_setting to work';
  },
  stackAlloc: function (size) { var ret = STACKTOP;STACKTOP = (STACKTOP + size)|0;STACKTOP = (((STACKTOP)+15)&-16);(assert((((STACKTOP|0) < (STACK_MAX|0))|0))|0); return ret; },
  staticAlloc: function (size) { var ret = STATICTOP;STATICTOP = (STATICTOP + (assert(!staticSealed),size))|0;STATICTOP = (((STATICTOP)+15)&-16); return ret; },
  dynamicAlloc: function (size) { assert(DYNAMICTOP_PTR);var ret = HEAP32[DYNAMICTOP_PTR>>2];var end = (((ret + size + 15)|0) & -16);HEAP32[DYNAMICTOP_PTR>>2] = end;if (end >= TOTAL_MEMORY) {var success = enlargeMemory();if (!success) {HEAP32[DYNAMICTOP_PTR>>2] = ret;return 0;}}return ret;},
  alignMemory: function (size,quantum) { var ret = size = Math.ceil((size)/(quantum ? quantum : 16))*(quantum ? quantum : 16); return ret; },
  makeBigInt: function (low,high,unsigned) { var ret = (unsigned ? ((+((low>>>0)))+((+((high>>>0)))*4294967296.0)) : ((+((low>>>0)))+((+((high|0)))*4294967296.0))); return ret; },
  GLOBAL_BASE: 8,
  QUANTUM_SIZE: 4,
  __dummy__: 0
}



Module["Runtime"] = Runtime;



//========================================
// Runtime essentials
//========================================

var ABORT = 0; // whether we are quitting the application. no code should run after this. set in exit() and abort()
var EXITSTATUS = 0;

function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

var globalScope = this;

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
  if (!func) {
    try { func = eval('_' + ident); } catch(e) {}
  }
  assert(func, 'Cannot call unknown function ' + ident + ' (perhaps LLVM optimizations or closure removed it?)');
  return func;
}

var cwrap, ccall;
(function(){
  var JSfuncs = {
    // Helpers for cwrap -- it can't refer to Runtime directly because it might
    // be renamed by closure, instead it calls JSfuncs['stackSave'].body to find
    // out what the minified function name is.
    'stackSave': function() {
      Runtime.stackSave()
    },
    'stackRestore': function() {
      Runtime.stackRestore()
    },
    // type conversion from js to c
    'arrayToC' : function(arr) {
      var ret = Runtime.stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    },
    'stringToC' : function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = Runtime.stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    }
  };
  // For fast lookup of conversion functions
  var toC = {'string' : JSfuncs['stringToC'], 'array' : JSfuncs['arrayToC']};

  // C calling interface.
  ccall = function ccallFunc(ident, returnType, argTypes, args, opts) {
    var func = getCFunc(ident);
    var cArgs = [];
    var stack = 0;
    assert(returnType !== 'array', 'Return type should not be "array".');
    if (args) {
      for (var i = 0; i < args.length; i++) {
        var converter = toC[argTypes[i]];
        if (converter) {
          if (stack === 0) stack = Runtime.stackSave();
          cArgs[i] = converter(args[i]);
        } else {
          cArgs[i] = args[i];
        }
      }
    }
    var ret = func.apply(null, cArgs);
    if ((!opts || !opts.async) && typeof EmterpreterAsync === 'object') {
      assert(!EmterpreterAsync.state, 'cannot start async op with normal JS calling ccall');
    }
    if (opts && opts.async) assert(!returnType, 'async ccalls cannot return values');
    if (returnType === 'string') ret = Pointer_stringify(ret);
    if (stack !== 0) {
      if (opts && opts.async) {
        EmterpreterAsync.asyncFinalizers.push(function() {
          Runtime.stackRestore(stack);
        });
        return;
      }
      Runtime.stackRestore(stack);
    }
    return ret;
  }

  var sourceRegex = /^function\s*[a-zA-Z$_0-9]*\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;
  function parseJSFunc(jsfunc) {
    // Match the body and the return value of a javascript function source
    var parsed = jsfunc.toString().match(sourceRegex).slice(1);
    return {arguments : parsed[0], body : parsed[1], returnValue: parsed[2]}
  }

  // sources of useful functions. we create this lazily as it can trigger a source decompression on this entire file
  var JSsource = null;
  function ensureJSsource() {
    if (!JSsource) {
      JSsource = {};
      for (var fun in JSfuncs) {
        if (JSfuncs.hasOwnProperty(fun)) {
          // Elements of toCsource are arrays of three items:
          // the code, and the return value
          JSsource[fun] = parseJSFunc(JSfuncs[fun]);
        }
      }
    }
  }

  cwrap = function cwrap(ident, returnType, argTypes) {
    argTypes = argTypes || [];
    var cfunc = getCFunc(ident);
    // When the function takes numbers and returns a number, we can just return
    // the original function
    var numericArgs = argTypes.every(function(type){ return type === 'number'});
    var numericRet = (returnType !== 'string');
    if ( numericRet && numericArgs) {
      return cfunc;
    }
    // Creation of the arguments list (["$1","$2",...,"$nargs"])
    var argNames = argTypes.map(function(x,i){return '$'+i});
    var funcstr = "(function(" + argNames.join(',') + ") {";
    var nargs = argTypes.length;
    if (!numericArgs) {
      // Generate the code needed to convert the arguments from javascript
      // values to pointers
      ensureJSsource();
      funcstr += 'var stack = ' + JSsource['stackSave'].body + ';';
      for (var i = 0; i < nargs; i++) {
        var arg = argNames[i], type = argTypes[i];
        if (type === 'number') continue;
        var convertCode = JSsource[type + 'ToC']; // [code, return]
        funcstr += 'var ' + convertCode.arguments + ' = ' + arg + ';';
        funcstr += convertCode.body + ';';
        funcstr += arg + '=(' + convertCode.returnValue + ');';
      }
    }

    // When the code is compressed, the name of cfunc is not literally 'cfunc' anymore
    var cfuncname = parseJSFunc(function(){return cfunc}).returnValue;
    // Call the function
    funcstr += 'var ret = ' + cfuncname + '(' + argNames.join(',') + ');';
    if (!numericRet) { // Return type can only by 'string' or 'number'
      // Convert the result to a string
      var strgfy = parseJSFunc(function(){return Pointer_stringify}).returnValue;
      funcstr += 'ret = ' + strgfy + '(ret);';
    }
    funcstr += "if (typeof EmterpreterAsync === 'object') { assert(!EmterpreterAsync.state, 'cannot start async op with normal JS calling cwrap') }";
    if (!numericArgs) {
      // If we had a stack, restore it
      ensureJSsource();
      funcstr += JSsource['stackRestore'].body.replace('()', '(stack)') + ';';
    }
    funcstr += 'return ret})';
    return eval(funcstr);
  };
})();
Module["ccall"] = ccall;
Module["cwrap"] = cwrap;

function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': HEAP8[((ptr)>>0)]=value; break;
      case 'i8': HEAP8[((ptr)>>0)]=value; break;
      case 'i16': HEAP16[((ptr)>>1)]=value; break;
      case 'i32': HEAP32[((ptr)>>2)]=value; break;
      case 'i64': (tempI64 = [value>>>0,(tempDouble=value,(+(Math_abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math_min((+(Math_floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math_ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[((ptr)>>2)]=tempI64[0],HEAP32[(((ptr)+(4))>>2)]=tempI64[1]); break;
      case 'float': HEAPF32[((ptr)>>2)]=value; break;
      case 'double': HEAPF64[((ptr)>>3)]=value; break;
      default: abort('invalid type for setValue: ' + type);
    }
}
Module["setValue"] = setValue;


function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': return HEAP8[((ptr)>>0)];
      case 'i8': return HEAP8[((ptr)>>0)];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP32[((ptr)>>2)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      default: abort('invalid type for setValue: ' + type);
    }
  return null;
}
Module["getValue"] = getValue;

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed
var ALLOC_DYNAMIC = 3; // Cannot be freed except through sbrk
var ALLOC_NONE = 4; // Do not allocate
Module["ALLOC_NORMAL"] = ALLOC_NORMAL;
Module["ALLOC_STACK"] = ALLOC_STACK;
Module["ALLOC_STATIC"] = ALLOC_STATIC;
Module["ALLOC_DYNAMIC"] = ALLOC_DYNAMIC;
Module["ALLOC_NONE"] = ALLOC_NONE;

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [typeof _malloc === 'function' ? _malloc : Runtime.staticAlloc, Runtime.stackAlloc, Runtime.staticAlloc, Runtime.dynamicAlloc][allocator === undefined ? ALLOC_STATIC : allocator](Math.max(size, singleType ? 1 : types.length));
  }

  if (zeroinit) {
    var ptr = ret, stop;
    assert((ret & 3) == 0);
    stop = ret + (size & ~3);
    for (; ptr < stop; ptr += 4) {
      HEAP32[((ptr)>>2)]=0;
    }
    stop = ret + size;
    while (ptr < stop) {
      HEAP8[((ptr++)>>0)]=0;
    }
    return ret;
  }

  if (singleType === 'i8') {
    if (slab.subarray || slab.slice) {
      HEAPU8.set(slab, ret);
    } else {
      HEAPU8.set(new Uint8Array(slab), ret);
    }
    return ret;
  }

  var i = 0, type, typeSize, previousType;
  while (i < size) {
    var curr = slab[i];

    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }

    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }
    assert(type, 'Must know what type to store in allocate!');

    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later

    setValue(ret+i, curr, type);

    // no need to look up size unless type changes, so cache it
    if (previousType !== type) {
      typeSize = Runtime.getNativeTypeSize(type);
      previousType = type;
    }
    i += typeSize;
  }

  return ret;
}
Module["allocate"] = allocate;

// Allocate memory during any stage of startup - static memory early on, dynamic memory later, malloc when ready
function getMemory(size) {
  if (!staticSealed) return Runtime.staticAlloc(size);
  if (!runtimeInitialized) return Runtime.dynamicAlloc(size);
  return _malloc(size);
}
Module["getMemory"] = getMemory;

function Pointer_stringify(ptr, /* optional */ length) {
  if (length === 0 || !ptr) return '';
  // TODO: use TextDecoder
  // Find the length, and check for UTF while doing so
  var hasUtf = 0;
  var t;
  var i = 0;
  while (1) {
    assert(ptr + i < TOTAL_MEMORY);
    t = HEAPU8[(((ptr)+(i))>>0)];
    hasUtf |= t;
    if (t == 0 && !length) break;
    i++;
    if (length && i == length) break;
  }
  if (!length) length = i;

  var ret = '';

  if (hasUtf < 128) {
    var MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
    var curr;
    while (length > 0) {
      curr = String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
      ret = ret ? ret + curr : curr;
      ptr += MAX_CHUNK;
      length -= MAX_CHUNK;
    }
    return ret;
  }
  return Module['UTF8ToString'](ptr);
}
Module["Pointer_stringify"] = Pointer_stringify;

// Given a pointer 'ptr' to a null-terminated ASCII-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function AsciiToString(ptr) {
  var str = '';
  while (1) {
    var ch = HEAP8[((ptr++)>>0)];
    if (!ch) return str;
    str += String.fromCharCode(ch);
  }
}
Module["AsciiToString"] = AsciiToString;

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in ASCII form. The copy will require at most str.length+1 bytes of space in the HEAP.

function stringToAscii(str, outPtr) {
  return writeAsciiToMemory(str, outPtr, false);
}
Module["stringToAscii"] = stringToAscii;

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the given array that contains uint8 values, returns
// a copy of that string as a Javascript String object.

var UTF8Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf8') : undefined;
function UTF8ArrayToString(u8Array, idx) {
  var endPtr = idx;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  while (u8Array[endPtr]) ++endPtr;

  if (endPtr - idx > 16 && u8Array.subarray && UTF8Decoder) {
    return UTF8Decoder.decode(u8Array.subarray(idx, endPtr));
  } else {
    var u0, u1, u2, u3, u4, u5;

    var str = '';
    while (1) {
      // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
      u0 = u8Array[idx++];
      if (!u0) return str;
      if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
      u1 = u8Array[idx++] & 63;
      if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
      u2 = u8Array[idx++] & 63;
      if ((u0 & 0xF0) == 0xE0) {
        u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
      } else {
        u3 = u8Array[idx++] & 63;
        if ((u0 & 0xF8) == 0xF0) {
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | u3;
        } else {
          u4 = u8Array[idx++] & 63;
          if ((u0 & 0xFC) == 0xF8) {
            u0 = ((u0 & 3) << 24) | (u1 << 18) | (u2 << 12) | (u3 << 6) | u4;
          } else {
            u5 = u8Array[idx++] & 63;
            u0 = ((u0 & 1) << 30) | (u1 << 24) | (u2 << 18) | (u3 << 12) | (u4 << 6) | u5;
          }
        }
      }
      if (u0 < 0x10000) {
        str += String.fromCharCode(u0);
      } else {
        var ch = u0 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      }
    }
  }
}
Module["UTF8ArrayToString"] = UTF8ArrayToString;

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function UTF8ToString(ptr) {
  return UTF8ArrayToString(HEAPU8,ptr);
}
Module["UTF8ToString"] = UTF8ToString;

// Copies the given Javascript String object 'str' to the given byte array at address 'outIdx',
// encoded in UTF8 form and null-terminated. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outU8Array: the array to copy to. Each index in this array is assumed to be one 8-byte element.
//   outIdx: The starting offset in the array to begin the copying.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=1, only the null terminator will be written and nothing else.
//                    maxBytesToWrite=0 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8Array(str, outU8Array, outIdx, maxBytesToWrite) {
  if (!(maxBytesToWrite > 0)) // Parameter maxBytesToWrite is not optional. Negative values, 0, null, undefined and false each don't write out any bytes.
    return 0;

  var startIdx = outIdx;
  var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) {
      if (outIdx >= endIdx) break;
      outU8Array[outIdx++] = u;
    } else if (u <= 0x7FF) {
      if (outIdx + 1 >= endIdx) break;
      outU8Array[outIdx++] = 0xC0 | (u >> 6);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0xFFFF) {
      if (outIdx + 2 >= endIdx) break;
      outU8Array[outIdx++] = 0xE0 | (u >> 12);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0x1FFFFF) {
      if (outIdx + 3 >= endIdx) break;
      outU8Array[outIdx++] = 0xF0 | (u >> 18);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0x3FFFFFF) {
      if (outIdx + 4 >= endIdx) break;
      outU8Array[outIdx++] = 0xF8 | (u >> 24);
      outU8Array[outIdx++] = 0x80 | ((u >> 18) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else {
      if (outIdx + 5 >= endIdx) break;
      outU8Array[outIdx++] = 0xFC | (u >> 30);
      outU8Array[outIdx++] = 0x80 | ((u >> 24) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 18) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    }
  }
  // Null-terminate the pointer to the buffer.
  outU8Array[outIdx] = 0;
  return outIdx - startIdx;
}
Module["stringToUTF8Array"] = stringToUTF8Array;

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF8 form. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8(str, outPtr, maxBytesToWrite) {
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  return stringToUTF8Array(str, HEAPU8,outPtr, maxBytesToWrite);
}
Module["stringToUTF8"] = stringToUTF8;

// Returns the number of bytes the given Javascript string takes if encoded as a UTF8 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF8(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) {
      ++len;
    } else if (u <= 0x7FF) {
      len += 2;
    } else if (u <= 0xFFFF) {
      len += 3;
    } else if (u <= 0x1FFFFF) {
      len += 4;
    } else if (u <= 0x3FFFFFF) {
      len += 5;
    } else {
      len += 6;
    }
  }
  return len;
}
Module["lengthBytesUTF8"] = lengthBytesUTF8;

// Given a pointer 'ptr' to a null-terminated UTF16LE-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

var UTF16Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf-16le') : undefined;
function UTF16ToString(ptr) {
  assert(ptr % 2 == 0, 'Pointer passed to UTF16ToString must be aligned to two bytes!');
  var endPtr = ptr;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  var idx = endPtr >> 1;
  while (HEAP16[idx]) ++idx;
  endPtr = idx << 1;

  if (endPtr - ptr > 32 && UTF16Decoder) {
    return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
  } else {
    var i = 0;

    var str = '';
    while (1) {
      var codeUnit = HEAP16[(((ptr)+(i*2))>>1)];
      if (codeUnit == 0) return str;
      ++i;
      // fromCharCode constructs a character from a UTF-16 code unit, so we can pass the UTF16 string right through.
      str += String.fromCharCode(codeUnit);
    }
  }
}


// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF16 form. The copy will require at most str.length*4+2 bytes of space in the HEAP.
// Use the function lengthBytesUTF16() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=2, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<2 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF16(str, outPtr, maxBytesToWrite) {
  assert(outPtr % 2 == 0, 'Pointer passed to stringToUTF16 must be aligned to two bytes!');
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 2) return 0;
  maxBytesToWrite -= 2; // Null terminator.
  var startPtr = outPtr;
  var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
  for (var i = 0; i < numCharsToWrite; ++i) {
    // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    HEAP16[((outPtr)>>1)]=codeUnit;
    outPtr += 2;
  }
  // Null-terminate the pointer to the HEAP.
  HEAP16[((outPtr)>>1)]=0;
  return outPtr - startPtr;
}


// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF16(str) {
  return str.length*2;
}


function UTF32ToString(ptr) {
  assert(ptr % 4 == 0, 'Pointer passed to UTF32ToString must be aligned to four bytes!');
  var i = 0;

  var str = '';
  while (1) {
    var utf32 = HEAP32[(((ptr)+(i*4))>>2)];
    if (utf32 == 0)
      return str;
    ++i;
    // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    if (utf32 >= 0x10000) {
      var ch = utf32 - 0x10000;
      str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
    } else {
      str += String.fromCharCode(utf32);
    }
  }
}


// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF32 form. The copy will require at most str.length*4+4 bytes of space in the HEAP.
// Use the function lengthBytesUTF32() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=4, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<4 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF32(str, outPtr, maxBytesToWrite) {
  assert(outPtr % 4 == 0, 'Pointer passed to stringToUTF32 must be aligned to four bytes!');
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 4) return 0;
  var startPtr = outPtr;
  var endPtr = startPtr + maxBytesToWrite - 4;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
      var trailSurrogate = str.charCodeAt(++i);
      codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
    }
    HEAP32[((outPtr)>>2)]=codeUnit;
    outPtr += 4;
    if (outPtr + 4 > endPtr) break;
  }
  // Null-terminate the pointer to the HEAP.
  HEAP32[((outPtr)>>2)]=0;
  return outPtr - startPtr;
}


// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF32(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i);
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
    len += 4;
  }

  return len;
}


function demangle(func) {
  var __cxa_demangle_func = Module['___cxa_demangle'] || Module['__cxa_demangle'];
  if (__cxa_demangle_func) {
    try {
      var s =
        func.substr(1);
      var len = lengthBytesUTF8(s)+1;
      var buf = _malloc(len);
      stringToUTF8(s, buf, len);
      var status = _malloc(4);
      var ret = __cxa_demangle_func(buf, 0, 0, status);
      if (getValue(status, 'i32') === 0 && ret) {
        return Pointer_stringify(ret);
      }
      // otherwise, libcxxabi failed
    } catch(e) {
      // ignore problems here
    } finally {
      if (buf) _free(buf);
      if (status) _free(status);
      if (ret) _free(ret);
    }
    // failure when using libcxxabi, don't demangle
    return func;
  }
  Runtime.warnOnce('warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling');
  return func;
}

function demangleAll(text) {
  var regex =
    /__Z[\w\d_]+/g;
  return text.replace(regex,
    function(x) {
      var y = demangle(x);
      return x === y ? x : (x + ' [' + y + ']');
    });
}

function jsStackTrace() {
  var err = new Error();
  if (!err.stack) {
    // IE10+ special cases: It does have callstack info, but it is only populated if an Error object is thrown,
    // so try that as a special-case.
    try {
      throw new Error(0);
    } catch(e) {
      err = e;
    }
    if (!err.stack) {
      return '(no stack trace available)';
    }
  }
  return err.stack.toString();
}

function stackTrace() {
  var js = jsStackTrace();
  if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
  return demangleAll(js);
}
Module["stackTrace"] = stackTrace;

// Memory management

var PAGE_SIZE = 16384;
var WASM_PAGE_SIZE = 65536;
var ASMJS_PAGE_SIZE = 16777216;
var MIN_TOTAL_MEMORY = 16777216;

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

var HEAP;
var buffer;
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

function updateGlobalBuffer(buf) {
  Module['buffer'] = buffer = buf;
}

function updateGlobalBufferViews() {
  Module['HEAP8'] = HEAP8 = new Int8Array(buffer);
  Module['HEAP16'] = HEAP16 = new Int16Array(buffer);
  Module['HEAP32'] = HEAP32 = new Int32Array(buffer);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buffer);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buffer);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buffer);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buffer);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buffer);
}

var STATIC_BASE, STATICTOP, staticSealed; // static area
var STACK_BASE, STACKTOP, STACK_MAX; // stack area
var DYNAMIC_BASE, DYNAMICTOP_PTR; // dynamic area handled by sbrk

  STATIC_BASE = STATICTOP = STACK_BASE = STACKTOP = STACK_MAX = DYNAMIC_BASE = DYNAMICTOP_PTR = 0;
  staticSealed = false;


// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  assert((STACK_MAX & 3) == 0);
  HEAPU32[(STACK_MAX >> 2)-1] = 0x02135467;
  HEAPU32[(STACK_MAX >> 2)-2] = 0x89BACDFE;
}

function checkStackCookie() {
  if (HEAPU32[(STACK_MAX >> 2)-1] != 0x02135467 || HEAPU32[(STACK_MAX >> 2)-2] != 0x89BACDFE) {
    abort('Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x' + HEAPU32[(STACK_MAX >> 2)-2].toString(16) + ' ' + HEAPU32[(STACK_MAX >> 2)-1].toString(16));
  }
  // Also test the global address 0 for integrity. This check is not compatible with SAFE_SPLIT_MEMORY though, since that mode already tests all address 0 accesses on its own.
  if (HEAP32[0] !== 0x63736d65 /* 'emsc' */) throw 'Runtime error: The application has corrupted its heap memory area (address zero)!';
}

function abortStackOverflow(allocSize) {
  abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - asm.stackSave() + allocSize) + ' bytes available!');
}

function abortOnCannotGrowMemory() {
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which adjusts the size at runtime but prevents some optimizations, (3) set Module.TOTAL_MEMORY to a higher value before the program runs, or if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
}


function enlargeMemory() {
  abortOnCannotGrowMemory();
}


var TOTAL_STACK = Module['TOTAL_STACK'] || 5242880;
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || 16777216;
if (TOTAL_MEMORY < TOTAL_STACK) Module.printErr('TOTAL_MEMORY should be larger than TOTAL_STACK, was ' + TOTAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// Initialize the runtime's memory
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && !!(new Int32Array(1)['subarray']) && !!(new Int32Array(1)['set']),
       'JS engine does not provide full typed array support');



// Use a provided buffer, if there is one, or else allocate a new one
if (Module['buffer']) {
  buffer = Module['buffer'];
  assert(buffer.byteLength === TOTAL_MEMORY, 'provided buffer should be ' + TOTAL_MEMORY + ' bytes, but it is ' + buffer.byteLength);
} else {
  // Use a WebAssembly memory where available
  {
    buffer = new ArrayBuffer(TOTAL_MEMORY);
  }
  assert(buffer.byteLength === TOTAL_MEMORY);
}
updateGlobalBufferViews();


function getTotalMemory() {
  return TOTAL_MEMORY;
}

// Endianness check (note: assumes compiler arch was little-endian)
  HEAP32[0] = 0x63736d65; /* 'emsc' */
HEAP16[1] = 0x6373;
if (HEAPU8[2] !== 0x73 || HEAPU8[3] !== 0x63) throw 'Runtime error: expected the system to be little-endian!';

Module['HEAP'] = HEAP;
Module['buffer'] = buffer;
Module['HEAP8'] = HEAP8;
Module['HEAP16'] = HEAP16;
Module['HEAP32'] = HEAP32;
Module['HEAPU8'] = HEAPU8;
Module['HEAPU16'] = HEAPU16;
Module['HEAPU32'] = HEAPU32;
Module['HEAPF32'] = HEAPF32;
Module['HEAPF64'] = HEAPF64;

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    if (typeof callback == 'function') {
      callback();
      continue;
    }
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Module['dynCall_v'](func);
      } else {
        Module['dynCall_vi'](func, callback.arg);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the runtime has exited

var runtimeInitialized = false;
var runtimeExited = false;


function preRun() {
  // compatibility - merge in anything from Module['preRun'] at this time
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPRERUN__);
}

function ensureInitRuntime() {
  checkStackCookie();
  if (runtimeInitialized) return;
  runtimeInitialized = true;
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
  checkStackCookie();
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
  checkStackCookie();
  callRuntimeCallbacks(__ATEXIT__);
  runtimeExited = true;
}

function postRun() {
  checkStackCookie();
  // compatibility - merge in anything from Module['postRun'] at this time
  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}
Module["addOnPreRun"] = addOnPreRun;

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}
Module["addOnInit"] = addOnInit;

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}
Module["addOnPreMain"] = addOnPreMain;

function addOnExit(cb) {
  __ATEXIT__.unshift(cb);
}
Module["addOnExit"] = addOnExit;

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}
Module["addOnPostRun"] = addOnPostRun;

// Tools


function intArrayFromString(stringy, dontAddNull, length /* optional */) {
  var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
}
Module["intArrayFromString"] = intArrayFromString;

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
      assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}
Module["intArrayToString"] = intArrayToString;

// Deprecated: This function should not be called because it is unsafe and does not provide
// a maximum length limit of how many bytes it is allowed to write. Prefer calling the
// function stringToUTF8Array() instead, which takes in a maximum length that can be used
// to be secure from out of bounds writes.
function writeStringToMemory(string, buffer, dontAddNull) {
  Runtime.warnOnce('writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!');

  var lastChar, end;
  if (dontAddNull) {
    // stringToUTF8Array always appends null. If we don't want to do that, remember the
    // character that existed at the location where the null will be placed, and restore
    // that after the write (below).
    end = buffer + lengthBytesUTF8(string);
    lastChar = HEAP8[end];
  }
  stringToUTF8(string, buffer, Infinity);
  if (dontAddNull) HEAP8[end] = lastChar; // Restore the value under the null character.
}
Module["writeStringToMemory"] = writeStringToMemory;

function writeArrayToMemory(array, buffer) {
  assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
  HEAP8.set(array, buffer);
}
Module["writeArrayToMemory"] = writeArrayToMemory;

function writeAsciiToMemory(str, buffer, dontAddNull) {
  for (var i = 0; i < str.length; ++i) {
    assert(str.charCodeAt(i) === str.charCodeAt(i)&0xff);
    HEAP8[((buffer++)>>0)]=str.charCodeAt(i);
  }
  // Null-terminate the pointer to the HEAP.
  if (!dontAddNull) HEAP8[((buffer)>>0)]=0;
}
Module["writeAsciiToMemory"] = writeAsciiToMemory;

function unSign(value, bits, ignore) {
  if (value >= 0) {
    return value;
  }
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
}
function reSign(value, bits, ignore) {
  if (value <= 0) {
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
  if (value >= half && (bits <= 32 || value > half)) { // for huge values, we can hit the precision limit and always get true here. so don't do that
                                                       // but, in general there is no perfect solution here. With 64-bit ints, we get rounding and errors
                                                       // TODO: In i64 mode 1, resign the two parts separately and safely
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
  return value;
}


// check for imul support, and also for correctness ( https://bugs.webkit.org/show_bug.cgi?id=126345 )
if (!Math['imul'] || Math['imul'](0xffffffff, 5) !== -5) Math['imul'] = function imul(a, b) {
  var ah  = a >>> 16;
  var al = a & 0xffff;
  var bh  = b >>> 16;
  var bl = b & 0xffff;
  return (al*bl + ((ah*bl + al*bh) << 16))|0;
};
Math.imul = Math['imul'];


if (!Math['clz32']) Math['clz32'] = function(x) {
  x = x >>> 0;
  for (var i = 0; i < 32; i++) {
    if (x & (1 << (31 - i))) return i;
  }
  return 32;
};
Math.clz32 = Math['clz32']

if (!Math['trunc']) Math['trunc'] = function(x) {
  return x < 0 ? Math.ceil(x) : Math.floor(x);
};
Math.trunc = Math['trunc'];

var Math_abs = Math.abs;
var Math_cos = Math.cos;
var Math_sin = Math.sin;
var Math_tan = Math.tan;
var Math_acos = Math.acos;
var Math_asin = Math.asin;
var Math_atan = Math.atan;
var Math_atan2 = Math.atan2;
var Math_exp = Math.exp;
var Math_log = Math.log;
var Math_sqrt = Math.sqrt;
var Math_ceil = Math.ceil;
var Math_floor = Math.floor;
var Math_pow = Math.pow;
var Math_imul = Math.imul;
var Math_fround = Math.fround;
var Math_round = Math.round;
var Math_min = Math.min;
var Math_clz32 = Math.clz32;
var Math_trunc = Math.trunc;

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// PRE_RUN_ADDITIONS (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};

function getUniqueRunDependency(id) {
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
  return id;
}

function addRunDependency(id) {
  runDependencies++;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            Module.printErr('still waiting on run dependencies:');
          }
          Module.printErr('dependency: ' + dep);
        }
        if (shown) {
          Module.printErr('(end of list)');
        }
      }, 10000);
    }
  } else {
    Module.printErr('warning: run dependency added without ID');
  }
}
Module["addRunDependency"] = addRunDependency;

function removeRunDependency(id) {
  runDependencies--;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    Module.printErr('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}
Module["removeRunDependency"] = removeRunDependency;

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data



var memoryInitializer = null;



var /* show errors on likely calls to FS when it was not included */ FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1');
  },
  init: function() { FS.error() },
  createDataFile: function() { FS.error() },
  createPreloadedFile: function() { FS.error() },
  createLazyFile: function() { FS.error() },
  open: function() { FS.error() },
  mkdev: function() { FS.error() },
  registerDevice: function() { FS.error() },
  analyzePath: function() { FS.error() },
  loadFilesFromDB: function() { FS.error() },

  ErrnoError: function ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;



// === Body ===

var ASM_CONSTS = [function($0, $1) { { Module.printErr('bad name in getProcAddress: ' + [Pointer_stringify($0), Pointer_stringify($1)]); } }];

function _emscripten_asm_const_iii(code, a0, a1) {
 return ASM_CONSTS[code](a0, a1);
}



STATIC_BASE = 8;

STATICTOP = STATIC_BASE + 19328;
  /* global initializers */  __ATINIT__.push();
  

/* memory initializer */ allocate([32,3,0,0,194,1,0,0,180,134,1,0,2,15,3,0,5,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,32,0,0,176,1,0,0,0,0,0,0,0,0,0,32,37,249,142,0,10,2,0,0,128,190,125,95,244,125,31,160,242,43,74,30,9,82,8,0,64,34,65,80,20,4,16,32,32,41,46,18,8,34,8,0,32,34,65,80,20,4,16,32,32,249,16,76,8,250,62,60,16,34,125,222,247,125,16,32,32,161,232,50,8,34,8,0,8,34,5,16,4,69,16,0,240,163,164,50,8,82,8,0,4,34,5,16,4,69,16,32,32,249,226,94,8,2,0,129,2,62,125,31,244,125,16,0,0,32,0,0,176,1,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,15,0,192,15,224,247,251,125,126,191,95,232,190,80,0,162,8,8,68,232,47,20,10,133,2,129,80,72,160,80,0,162,40,228,73,40,40,20,10,132,2,129,64,72,160,72,0,190,15,2,16,175,235,247,9,132,62,159,216,79,160,71,0,34,136,228,9,161,42,20,10,132,2,129,80,72,160,72,0,34,40,8,4,160,47,20,10,133,2,129,80,72,162,80,0,190,143,0,0,33,32,244,251,125,126,129,95,232,156,208,7,0,128,0,0,224,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,1,12,0,130,66,191,223,239,247,251,11,5,5,133,66,191,4,72,0,198,66,161,80,40,20,64,8,5,37,133,66,160,8,168,0,170,70,161,80,40,20,64,8,5,37,133,66,144,16,8,0,146,74,161,95,232,247,67,8,5,37,121,126,136,32,8,0,130,82,161,64,40,1,66,8,137,36,133,64,132,64,8,0,130,98,161,64,42,2,66,8,81,36,133,64,130,128,8,0,130,66,191,192,47,244,67,248,33,252,133,126,191,0,9,62,0,0,0,0,4,0,0,0,0,0,0,0,128,1,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,4,0,32,72,65,0,0,0,0,0,8,0,0,4,4,0,4,60,32,0,65,0,0,0,0,0,8,0,0,240,125,223,247,133,239,75,81,190,239,251,190,239,59,81,4,0,69,65,20,133,40,74,73,170,40,138,162,32,8,81,4,240,69,65,244,157,40,74,71,170,40,138,162,224,11,81,4,16,69,65,20,132,40,74,73,170,40,138,162,0,10,145,2,240,125,223,247,133,47,74,209,170,232,251,190,224,123,31,1,0,0,0,0,4,8,64,0,0,0,8,32,0,0,0,0,0,0,0,0,132,15,96,0,0,0,8,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,1,15,0,0,0,0,0,0,0,0,0,0,0,0,0,36,1,9,0,0,0,0,0,0,0,0,0,6,0,0,0,36,1,9,0,0,0,0,0,0,0,128,16,9,162,40,250,36,1,9,0,0,0,0,0,0,0,0,62,1,42,37,66,34,82,9,0,0,0,0,0,0,0,128,138,3,42,34,34,36,41,9,0,0,0,0,0,0,0,128,10,1,42,37,18,36,1,9,0,0,0,0,0,0,0,128,10,1,190,232,251,36,1,9,0,0,0,0,0,0,0,128,190,14,0,0,2,172,1,15,0,0,0,0,0,0,0,128,4,0,0,224,3,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,0,0,0,14,184,67,132,3,58,32,0,128,160,190,2,32,0,0,240,138,32,82,196,2,43,32,4,34,145,2,248,59,0,240,7,142,56,75,228,2,58,32,2,28,138,30,8,42,233,17,4,224,11,66,244,2,130,36,1,20,4,20,232,186,4,209,5,128,184,195,231,10,58,137,0,28,14,60,40,2,9,80,4,128,0,64,196,2,128,68,0,34,132,32,232,2,0,80,4,0,0,64,128,2,0,32,5,0,142,62,8,2,0,16,4,224,3,64,128,66,0,0,7,0,132,0,248,3,0,240,7,0,0,64,128,34,0,0,4,0,0,0,0,0,0,0,0,0,0,64,128,2,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,7,128,0,194,160,72,24,0,0,1,132,33,9,146,2,66,38,4,1,33,81,0,0,127,63,2,66,2,16,41,0,34,20,192,239,247,251,253,126,9,161,223,239,247,187,187,3,18,15,68,40,20,10,133,66,9,129,64,32,16,16,17,1,8,4,68,40,20,10,133,66,127,129,64,32,16,16,17,1,4,130,199,239,247,251,253,126,9,129,207,231,243,17,17,1,50,169,80,40,20,10,133,66,9,161,64,32,16,16,17,1,64,184,80,40,20,10,133,66,121,191,223,239,247,187,187,3,32,160,31,0,0,0,0,0,0,16,0,0,0,0,0,0,112,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,2,8,131,34,1,0,2,8,67,2,1,0,1,1,124,20,4,132,68,1,0,32,4,132,4,128,8,63,130,0,132,66,191,223,239,247,3,126,161,80,40,20,10,33,0,0,132,70,161,80,40,20,138,82,161,80,40,20,122,161,239,3,158,74,161,80,40,20,82,82,161,80,40,20,74,31,8,2,132,82,161,80,40,20,34,74,161,80,40,244,75,161,239,3,132,98,161,80,40,20,82,74,161,80,40,4,122,161,40,2,124,66,191,223,239,247,139,126,191,223,239,247,11,189,239,3,0,0,0,0,0,0,0,4,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,5,32,0,0,4,132,0,34,129,69,17,16,66,1,0,148,66,81,0,0,8,66,81,148,42,162,32,8,165,80,0,0,0,32,0,0,0,0,0,0,0,5,0,0,0,0,8,190,239,251,254,251,190,239,251,20,145,235,251,190,239,251,0,32,8,130,32,10,162,40,138,20,145,40,138,162,40,138,62,190,239,251,254,11,190,239,251,20,145,40,138,162,40,138,0,162,40,138,34,8,130,32,8,20,145,40,138,162,40,138,8,190,239,251,254,251,190,239,251,20,145,47,250,190,239,251,0,0,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,33,0,4,0,0,0,0,0,0,0,0,0,0,0,0,130,80,20,2,20,0,0,0,0,0,0,0,0,0,0,16,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,190,40,138,162,40,34,0,0,0,0,0,0,0,0,0,0,170,40,138,162,232,34,0,0,0,0,0,0,0,0,0,0,170,40,138,162,168,34,0,0,0,0,0,0,0,0,0,0,170,40,138,162,232,34,0,0,0,0,0,0,0,0,0,0,190,239,251,190,47,62,0,0,0,0,0,0,0,0,0,0,4,0,0,0,40,32,0,0,0,0,0,0,0,0,0,0,0,0,0,128,15,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,4,0,0,0,6,0,0,0,5,0,0,0,7,0,0,0,6,0,0,0,2,0,0,0,3,0,0,0,3,0,0,0,5,0,0,0,5,0,0,0,2,0,0,0,4,0,0,0,1,0,0,0,7,0,0,0,5,0,0,0,2,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,1,0,0,0,1,0,0,0,3,0,0,0,4,0,0,0,3,0,0,0,6,0,0,0,7,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,3,0,0,0,5,0,0,0,6,0,0,0,5,0,0,0,7,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,7,0,0,0,6,0,0,0,7,0,0,0,7,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,2,0,0,0,7,0,0,0,2,0,0,0,3,0,0,0,5,0,0,0,2,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,4,0,0,0,5,0,0,0,5,0,0,0,1,0,0,0,2,0,0,0,5,0,0,0,2,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,4,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,3,0,0,0,1,0,0,0,3,0,0,0,4,0,0,0,4,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,7,0,0,0,1,0,0,0,5,0,0,0,3,0,0,0,7,0,0,0,3,0,0,0,5,0,0,0,4,0,0,0,1,0,0,0,7,0,0,0,4,0,0,0,3,0,0,0,5,0,0,0,3,0,0,0,3,0,0,0,2,0,0,0,5,0,0,0,6,0,0,0,1,0,0,0,2,0,0,0,2,0,0,0,3,0,0,0,5,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,7,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,3,0,0,0,3,0,0,0,3,0,0,0,3,0,0,0,7,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,5,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,6,0,0,0,4,0,0,0,6,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,9,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,2,0,0,0,2,0,0,0,3,0,0,0,3,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,3,0,0,0,5,0,0,0,255,255,255,255,0,1,0,0,255,255,255,255,0,0,128,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,168,12,0,0,5,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,115,71,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,168,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,114,97,121,108,105,98,32,91,116,101,120,116,93,32,101,120,97,109,112,108,101,32,45,32,116,101,120,116,32,102,111,114,109,97,116,116,105,110,103,0,83,99,111,114,101,58,32,37,48,56,105,0,72,105,83,99,111,114,101,58,32,37,48,56,105,0,76,105,118,101,115,58,32,37,48,50,105,0,69,108,97,112,115,101,100,32,84,105,109,101,58,32,37,48,50,46,48,50,102,32,109,115,0,73,110,105,116,105,97,108,105,122,105,110,103,32,114,97,121,108,105,98,32,40,118,49,46,55,46,48,41,0,35,99,97,110,118,97,115,0,84,97,114,103,101,116,32,116,105,109,101,32,112,101,114,32,102,114,97,109,101,58,32,37,48,50,46,48,51,102,32,109,105,108,108,105,115,101,99,111,110,100,115,0,69,115,99,97,112,101,0,67,97,110,118,97,115,32,115,99,97,108,101,100,32,116,111,32,102,117,108,108,115,99,114,101,101,110,46,32,69,108,101,109,101,110,116,83,105,122,101,58,32,40,37,105,120,37,105,41,44,32,83,99,114,101,101,110,83,105,122,101,40,37,105,120,37,105,41,0,67,97,110,118,97,115,32,115,99,97,108,101,100,32,116,111,32,119,105,110,100,111,119,101,100,46,32,69,108,101,109,101,110,116,83,105,122,101,58,32,40,37,105,120,37,105,41,44,32,83,99,114,101,101,110,83,105,122,101,40,37,105,120,37,105,41,0,91,84,69,88,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,102,111,110,116,32,108,111,97,100,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,68,88,84,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,69,84,67,49,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,69,84,67,50,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,80,86,82,84,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,65,83,84,67,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,84,101,120,116,117,114,101,32,102,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,91,84,69,88,32,73,68,32,37,105,93,32,84,101,120,116,117,114,101,32,99,114,101,97,116,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,37,105,120,37,105,41,0,84,101,120,116,117,114,101,32,99,111,117,108,100,32,110,111,116,32,98,101,32,99,114,101,97,116,101,100,0,73,109,97,103,101,32,100,97,116,97,32,102,111,114,109,97,116,32,105,115,32,99,111,109,112,114,101,115,115,101,100,44,32,99,97,110,32,110,111,116,32,98,101,32,99,111,110,118,101,114,116,101,100,0,70,111,114,109,97,116,32,110,111,116,32,115,117,112,112,111,114,116,101,100,32,102,111,114,32,112,105,120,101,108,32,100,97,116,97,32,114,101,116,114,105,101,118,97,108,0,70,97,105,108,101,100,32,116,111,32,105,110,105,116,105,97,108,105,122,101,32,71,76,70,87,0,84,114,121,105,110,103,32,116,111,32,101,110,97,98,108,101,32,77,83,65,65,32,120,52,0,67,108,111,115,101,115,116,32,102,117,108,108,115,99,114,101,101,110,32,118,105,100,101,111,109,111,100,101,58,32,37,105,32,120,32,37,105,0,71,76,70,87,32,70,97,105,108,101,100,32,116,111,32,105,110,105,116,105,97,108,105,122,101,32,87,105,110,100,111,119,0,68,105,115,112,108,97,121,32,100,101,118,105,99,101,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,82,101,110,100,101,114,32,115,105,122,101,58,32,37,105,32,120,32,37,105,0,83,99,114,101,101,110,32,115,105,122,101,58,32,37,105,32,120,32,37,105,0,86,105,101,119,112,111,114,116,32,111,102,102,115,101,116,115,58,32,37,105,44,32,37,105,0,84,114,121,105,110,103,32,116,111,32,101,110,97,98,108,101,32,86,83,89,78,67,0,71,80,85,58,32,86,101,110,100,111,114,58,32,32,32,37,115,0,71,80,85,58,32,82,101,110,100,101,114,101,114,58,32,37,115,0,71,80,85,58,32,86,101,114,115,105,111,110,58,32,32,37,115,0,71,80,85,58,32,71,76,83,76,58,32,32,32,32,32,37,115,0,32,0,78,117,109,98,101,114,32,111,102,32,115,117,112,112,111,114,116,101,100,32,101,120,116,101,110,115,105,111,110,115,58,32,37,105,0,71,76,95,79,69,83,95,118,101,114,116,101,120,95,97,114,114,97,121,95,111,98,106,101,99,116,0,103,108,71,101,110,86,101,114,116,101,120,65,114,114,97,121,115,79,69,83,0,103,108,66,105,110,100,86,101,114,116,101,120,65,114,114,97,121,79,69,83,0,103,108,68,101,108,101,116,101,86,101,114,116,101,120,65,114,114,97,121,115,79,69,83,0,71,76,95,79,69,83,95,116,101,120,116,117,114,101,95,110,112,111,116,0,71,76,95,69,88,84,95,116,101,120,116,117,114,101,95,99,111,109,112,114,101,115,115,105,111,110,95,115,51,116,99,0,71,76,95,87,69,66,71,76,95,99,111,109,112,114,101,115,115,101,100,95,116,101,120,116,117,114,101,95,115,51,116,99,0,71,76,95,87,69,66,75,73,84,95,87,69,66,71,76,95,99,111,109,112,114,101,115,115,101,100,95,116,101,120,116,117,114,101,95,115,51,116,99,0,71,76,95,79,69,83,95,99,111,109,112,114,101,115,115,101,100,95,69,84,67,49,95,82,71,66,56,95,116,101,120,116,117,114,101,0,71,76,95,87,69,66,71,76,95,99,111,109,112,114,101,115,115,101,100,95,116,101,120,116,117,114,101,95,101,116,99,49,0,71,76,95,65,82,66,95,69,83,51,95,99,111,109,112,97,116,105,98,105,108,105,116,121,0,71,76,95,73,77,71,95,116,101,120,116,117,114,101,95,99,111,109,112,114,101,115,115,105,111,110,95,112,118,114,116,99,0,71,76,95,75,72,82,95,116,101,120,116,117,114,101,95,99,111,109,112,114,101,115,115,105,111,110,95,97,115,116,99,95,104,100,114,0,71,76,95,69,88,84,95,116,101,120,116,117,114,101,95,102,105,108,116,101,114,95,97,110,105,115,111,116,114,111,112,105,99,0,71,76,95,69,88,84,95,116,101,120,116,117,114,101,95,109,105,114,114,111,114,95,99,108,97,109,112,0,91,69,88,84,69,78,83,73,79,78,93,32,86,65,79,32,101,120,116,101,110,115,105,111,110,32,100,101,116,101,99,116,101,100,44,32,86,65,79,32,102,117,110,99,116,105,111,110,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,69,88,84,69,78,83,73,79,78,93,32,86,65,79,32,101,120,116,101,110,115,105,111,110,32,110,111,116,32,102,111,117,110,100,44,32,86,65,79,32,117,115,97,103,101,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,78,80,79,84,32,116,101,120,116,117,114,101,115,32,101,120,116,101,110,115,105,111,110,32,100,101,116,101,99,116,101,100,44,32,102,117,108,108,32,78,80,79,84,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,78,80,79,84,32,116,101,120,116,117,114,101,115,32,101,120,116,101,110,115,105,111,110,32,110,111,116,32,102,111,117,110,100,44,32,108,105,109,105,116,101,100,32,78,80,79,84,32,115,117,112,112,111,114,116,32,40,110,111,45,109,105,112,109,97,112,115,44,32,110,111,45,114,101,112,101,97,116,41,0,91,69,88,84,69,78,83,73,79,78,93,32,68,88,84,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,69,84,67,49,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,69,84,67,50,47,69,65,67,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,80,86,82,84,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,65,83,84,67,32,99,111,109,112,114,101,115,115,101,100,32,116,101,120,116,117,114,101,115,32,115,117,112,112,111,114,116,101,100,0,91,69,88,84,69,78,83,73,79,78,93,32,65,110,105,115,111,116,114,111,112,105,99,32,116,101,120,116,117,114,101,115,32,102,105,108,116,101,114,105,110,103,32,115,117,112,112,111,114,116,101,100,32,40,109,97,120,58,32,37,46,48,102,88,41,0,91,69,88,84,69,78,83,73,79,78,93,32,67,108,97,109,112,32,109,105,114,114,111,114,32,119,114,97,112,32,116,101,120,116,117,114,101,32,109,111,100,101,32,115,117,112,112,111,114,116,101,100,0,91,84,69,88,32,73,68,32,37,105,93,32,66,97,115,101,32,119,104,105,116,101,32,116,101,120,116,117,114,101,32,108,111,97,100,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,66,97,115,101,32,119,104,105,116,101,32,116,101,120,116,117,114,101,32,99,111,117,108,100,32,110,111,116,32,98,101,32,108,111,97,100,101,100,0,79,112,101,110,71,76,32,100,101,102,97,117,108,116,32,115,116,97,116,101,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,67,80,85,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,108,105,110,101,115,44,32,116,114,105,97,110,103,108,101,115,44,32,113,117,97,100,115,41,0,91,86,65,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,65,79,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,108,105,110,101,115,41,0,91,86,66,79,32,73,68,32,37,105,93,91,86,66,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,66,79,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,108,105,110,101,115,41,0,91,86,65,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,65,79,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,116,114,105,97,110,103,108,101,115,41,0,91,86,66,79,32,73,68,32,37,105,93,91,86,66,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,66,79,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,116,114,105,97,110,103,108,101,115,41,0,91,86,65,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,65,79,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,113,117,97,100,115,41,0,91,86,66,79,32,73,68,32,37,105,93,91,86,66,79,32,73,68,32,37,105,93,91,86,66,79,32,73,68,32,37,105,93,91,86,66,79,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,98,117,102,102,101,114,115,32,86,66,79,115,32,105,110,105,116,105,97,108,105,122,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,32,40,113,117,97,100,115,41,0,35,118,101,114,115,105,111,110,32,49,48,48,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,97,116,116,114,105,98,117,116,101,32,118,101,99,51,32,118,101,114,116,101,120,80,111,115,105,116,105,111,110,59,32,32,32,32,32,10,97,116,116,114,105,98,117,116,101,32,118,101,99,50,32,118,101,114,116,101,120,84,101,120,67,111,111,114,100,59,32,32,32,32,32,10,97,116,116,114,105,98,117,116,101,32,118,101,99,52,32,118,101,114,116,101,120,67,111,108,111,114,59,32,32,32,32,32,32,32,32,10,118,97,114,121,105,110,103,32,118,101,99,50,32,102,114,97,103,84,101,120,67,111,111,114,100,59,32,32,32,32,32,32,32,32,32,10,118,97,114,121,105,110,103,32,118,101,99,52,32,102,114,97,103,67,111,108,111,114,59,32,32,32,32,32,32,32,32,32,32,32,32,10,117,110,105,102,111,114,109,32,109,97,116,52,32,109,118,112,77,97,116,114,105,120,59,32,32,32,32,32,32,32,32,32,32,32,32,10,118,111,105,100,32,109,97,105,110,40,41,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,123,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,102,114,97,103,84,101,120,67,111,111,114,100,32,61,32,118,101,114,116,101,120,84,101,120,67,111,111,114,100,59,32,10,32,32,32,32,102,114,97,103,67,111,108,111,114,32,61,32,118,101,114,116,101,120,67,111,108,111,114,59,32,32,32,32,32,32,32,10,32,32,32,32,103,108,95,80,111,115,105,116,105,111,110,32,61,32,109,118,112,77,97,116,114,105,120,42,118,101,99,52,40,118,101,114,116,101,120,80,111,115,105,116,105,111,110,44,32,49,46,48,41,59,32,10,125,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,0,35,118,101,114,115,105,111,110,32,49,48,48,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,112,114,101,99,105,115,105,111,110,32,109,101,100,105,117,109,112,32,102,108,111,97,116,59,32,32,32,32,32,32,32,32,32,32,32,10,118,97,114,121,105,110,103,32,118,101,99,50,32,102,114,97,103,84,101,120,67,111,111,114,100,59,32,32,32,32,32,32,32,32,32,10,118,97,114,121,105,110,103,32,118,101,99,52,32,102,114,97,103,67,111,108,111,114,59,32,32,32,32,32,32,32,32,32,32,32,32,10,117,110,105,102,111,114,109,32,115,97,109,112,108,101,114,50,68,32,116,101,120,116,117,114,101,48,59,32,32,32,32,32,32,32,32,10,117,110,105,102,111,114,109,32,118,101,99,52,32,99,111,108,68,105,102,102,117,115,101,59,32,32,32,32,32,32,32,32,32,32,32,10,118,111,105,100,32,109,97,105,110,40,41,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,123,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,118,101,99,52,32,116,101,120,101,108,67,111,108,111,114,32,61,32,116,101,120,116,117,114,101,50,68,40,116,101,120,116,117,114,101,48,44,32,102,114,97,103,84,101,120,67,111,111,114,100,41,59,32,10,32,32,32,32,103,108,95,70,114,97,103,67,111,108,111,114,32,61,32,116,101,120,101,108,67,111,108,111,114,42,99,111,108,68,105,102,102,117,115,101,42,102,114,97,103,67,111,108,111,114,59,32,32,32,32,32,32,10,125,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,0,91,83,72,68,82,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,115,104,97,100,101,114,32,108,111,97,100,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,83,72,68,82,32,73,68,32,37,105,93,32,68,101,102,97,117,108,116,32,115,104,97,100,101,114,32,99,111,117,108,100,32,110,111,116,32,98,101,32,108,111,97,100,101,100,0,118,101,114,116,101,120,80,111,115,105,116,105,111,110,0,118,101,114,116,101,120,84,101,120,67,111,111,114,100,0,118,101,114,116,101,120,84,101,120,67,111,111,114,100,50,0,118,101,114,116,101,120,78,111,114,109,97,108,0,118,101,114,116,101,120,84,97,110,103,101,110,116,0,118,101,114,116,101,120,67,111,108,111,114,0,109,118,112,77,97,116,114,105,120,0,99,111,108,68,105,102,102,117,115,101,0,99,111,108,65,109,98,105,101,110,116,0,99,111,108,83,112,101,99,117,108,97,114,0,116,101,120,116,117,114,101,48,0,116,101,120,116,117,114,101,49,0,116,101,120,116,117,114,101,50,0,91,86,83,72,68,82,32,73,68,32,37,105,93,32,70,97,105,108,101,100,32,116,111,32,99,111,109,112,105,108,101,32,118,101,114,116,101,120,32,115,104,97,100,101,114,46,46,46,0,37,115,0,91,86,83,72,68,82,32,73,68,32,37,105,93,32,86,101,114,116,101,120,32,115,104,97,100,101,114,32,99,111,109,112,105,108,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,70,83,72,68,82,32,73,68,32,37,105,93,32,70,97,105,108,101,100,32,116,111,32,99,111,109,112,105,108,101,32,102,114,97,103,109,101,110,116,32,115,104,97,100,101,114,46,46,46,0,91,70,83,72,68,82,32,73,68,32,37,105,93,32,70,114,97,103,109,101,110,116,32,115,104,97,100,101,114,32,99,111,109,112,105,108,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,83,72,68,82,32,73,68,32,37,105,93,32,70,97,105,108,101,100,32,116,111,32,108,105,110,107,32,115,104,97,100,101,114,32,112,114,111,103,114,97,109,46,46,46,0,91,83,72,68,82,32,73,68,32,37,105,93,32,83,104,97,100,101,114,32,112,114,111,103,114,97,109,32,108,111,97,100,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,68,79,87,78,83,67,65,76,73,78,71,58,32,82,101,113,117,105,114,101,100,32,115,99,114,101,101,110,32,115,105,122,101,32,40,37,105,120,37,105,41,32,105,115,32,98,105,103,103,101,114,32,116,104,97,110,32,100,105,115,112,108,97,121,32,115,105,122,101,32,40,37,105,120,37,105,41,0,68,111,119,110,115,99,97,108,101,32,109,97,116,114,105,120,32,103,101,110,101,114,97,116,101,100,44,32,99,111,110,116,101,110,116,32,119,105,108,108,32,98,101,32,114,101,110,100,101,114,101,100,32,97,116,58,32,37,105,32,120,32,37,105,0,85,80,83,67,65,76,73,78,71,58,32,82,101,113,117,105,114,101,100,32,115,99,114,101,101,110,32,115,105,122,101,58,32,37,105,32,120,32,37,105,32,45,62,32,68,105,115,112,108,97,121,32,115,105,122,101,58,32,37,105,32,120,32,37,105,0,91,71,76,70,87,51,32,69,114,114,111,114,93,32,67,111,100,101,58,32,37,105,32,68,101,99,114,105,112,116,105,111,110,58,32,37,115,0,73,78,70,79,58,32,0,87,65,82,78,73,78,71,58,32,0,87,105,110,100,111,119,32,99,108,111,115,101,100,32,115,117,99,99,101,115,115,102,117,108,108,121,0,91,84,69,88,32,73,68,32,37,105,93,32,85,110,108,111,97,100,101,100,32,116,101,120,116,117,114,101,32,100,97,116,97,32,40,98,97,115,101,32,119,104,105,116,101,32,116,101,120,116,117,114,101,41,32,102,114,111,109,32,86,82,65,77,0,91,84,69,88,32,73,68,32,37,105,93,32,85,110,108,111,97,100,101,100,32,116,101,120,116,117,114,101,32,100,97,116,97,32,102,114,111,109,32,86,82,65,77,32,40,71,80,85,41,0,83,116,97,99,107,32,66,117,102,102,101,114,32,79,118,101,114,102,108,111,119,32,40,77,65,88,32,37,105,32,77,97,116,114,105,120,41,0,77,65,88,95,76,73,78,69,83,95,66,65,84,67,72,32,111,118,101,114,102,108,111,119,0,77,65,88,95,84,82,73,65,78,71,76,69,83,95,66,65,84,67,72,32,111,118,101,114,102,108,111,119,0,77,65,88,95,81,85,65,68,83,95,66,65,84,67,72,32,111,118,101,114,102,108,111,119,0,69,88,84,0,65,82,66,0,79,69,83,0,65,78,71,76,69,0,103,108,67,114,101,97,116,101,80,114,111,103,114,97,109,79,98,106,101,99,116,0,103,108,67,114,101,97,116,101,80,114,111,103,114,97,109,0,103,108,85,115,101,80,114,111,103,114,97,109,79,98,106,101,99,116,0,103,108,85,115,101,80,114,111,103,114,97,109,0,103,108,67,114,101,97,116,101,83,104,97,100,101,114,79,98,106,101,99,116,0,103,108,67,114,101,97,116,101,83,104,97,100,101,114,0,103,108,65,116,116,97,99,104,79,98,106,101,99,116,0,103,108,65,116,116,97,99,104,83,104,97,100,101,114,0,103,108,68,101,116,97,99,104,79,98,106,101,99,116,0,103,108,68,101,116,97,99,104,83,104,97,100,101,114,0,103,108,80,105,120,101,108,83,116,111,114,101,105,0,103,108,71,101,116,83,116,114,105,110,103,0,103,108,71,101,116,73,110,116,101,103,101,114,118,0,103,108,71,101,116,70,108,111,97,116,118,0,103,108,71,101,116,66,111,111,108,101,97,110,118,0,103,108,71,101,110,84,101,120,116,117,114,101,115,0,103,108,68,101,108,101,116,101,84,101,120,116,117,114,101,115,0,103,108,67,111,109,112,114,101,115,115,101,100,84,101,120,73,109,97,103,101,50,68,0,103,108,67,111,109,112,114,101,115,115,101,100,84,101,120,83,117,98,73,109,97,103,101,50,68,0,103,108,84,101,120,73,109,97,103,101,50,68,0,103,108,84,101,120,83,117,98,73,109,97,103,101,50,68,0,103,108,82,101,97,100,80,105,120,101,108,115,0,103,108,66,105,110,100,84,101,120,116,117,114,101,0,103,108,71,101,116,84,101,120,80,97,114,97,109,101,116,101,114,102,118,0,103,108,71,101,116,84,101,120,80,97,114,97,109,101,116,101,114,105,118,0,103,108,84,101,120,80,97,114,97,109,101,116,101,114,102,118,0,103,108,84,101,120,80,97,114,97,109,101,116,101,114,105,118,0,103,108,73,115,84,101,120,116,117,114,101,0,103,108,71,101,110,66,117,102,102,101,114,115,0,103,108,68,101,108,101,116,101,66,117,102,102,101,114,115,0,103,108,71,101,116,66,117,102,102,101,114,80,97,114,97,109,101,116,101,114,105,118,0,103,108,66,117,102,102,101,114,68,97,116,97,0,103,108,66,117,102,102,101,114,83,117,98,68,97,116,97,0,103,108,73,115,66,117,102,102,101,114,0,103,108,71,101,110,82,101,110,100,101,114,98,117,102,102,101,114,115,0,103,108,68,101,108,101,116,101,82,101,110,100,101,114,98,117,102,102,101,114,115,0,103,108,66,105,110,100,82,101,110,100,101,114,98,117,102,102,101,114,0,103,108,71,101,116,82,101,110,100,101,114,98,117,102,102,101,114,80,97,114,97,109,101,116,101,114,105,118,0,103,108,73,115,82,101,110,100,101,114,98,117,102,102,101,114,0,103,108,71,101,116,85,110,105,102,111,114,109,102,118,0,103,108,71,101,116,85,110,105,102,111,114,109,105,118,0,103,108,71,101,116,85,110,105,102,111,114,109,76,111,99,97,116,105,111,110,0,103,108,71,101,116,86,101,114,116,101,120,65,116,116,114,105,98,102,118,0,103,108,71,101,116,86,101,114,116,101,120,65,116,116,114,105,98,105,118,0,103,108,71,101,116,86,101,114,116,101,120,65,116,116,114,105,98,80,111,105,110,116,101,114,118,0,103,108,71,101,116,65,99,116,105,118,101,85,110,105,102,111,114,109,0,103,108,85,110,105,102,111,114,109,49,102,0,103,108,85,110,105,102,111,114,109,50,102,0,103,108,85,110,105,102,111,114,109,51,102,0,103,108,85,110,105,102,111,114,109,52,102,0,103,108,85,110,105,102,111,114,109,49,105,0,103,108,85,110,105,102,111,114,109,50,105,0,103,108,85,110,105,102,111,114,109,51,105,0,103,108,85,110,105,102,111,114,109,52,105,0,103,108,85,110,105,102,111,114,109,49,105,118,0,103,108,85,110,105,102,111,114,109,50,105,118,0,103,108,85,110,105,102,111,114,109,51,105,118,0,103,108,85,110,105,102,111,114,109,52,105,118,0,103,108,85,110,105,102,111,114,109,49,102,118,0,103,108,85,110,105,102,111,114,109,50,102,118,0,103,108,85,110,105,102,111,114,109,51,102,118,0,103,108,85,110,105,102,111,114,109,52,102,118,0,103,108,85,110,105,102,111,114,109,77,97,116,114,105,120,50,102,118,0,103,108,85,110,105,102,111,114,109,77,97,116,114,105,120,51,102,118,0,103,108,85,110,105,102,111,114,109,77,97,116,114,105,120,52,102,118,0,103,108,66,105,110,100,66,117,102,102,101,114,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,49,102,118,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,50,102,118,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,51,102,118,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,52,102,118,0,103,108,71,101,116,65,116,116,114,105,98,76,111,99,97,116,105,111,110,0,103,108,71,101,116,65,99,116,105,118,101,65,116,116,114,105,98,0,103,108,68,101,108,101,116,101,83,104,97,100,101,114,0,103,108,71,101,116,65,116,116,97,99,104,101,100,83,104,97,100,101,114,115,0,103,108,83,104,97,100,101,114,83,111,117,114,99,101,0,103,108,71,101,116,83,104,97,100,101,114,83,111,117,114,99,101,0,103,108,67,111,109,112,105,108,101,83,104,97,100,101,114,0,103,108,71,101,116,83,104,97,100,101,114,73,110,102,111,76,111,103,0,103,108,71,101,116,83,104,97,100,101,114,105,118,0,103,108,71,101,116,80,114,111,103,114,97,109,105,118,0,103,108,73,115,83,104,97,100,101,114,0,103,108,68,101,108,101,116,101,80,114,111,103,114,97,109,0,103,108,71,101,116,83,104,97,100,101,114,80,114,101,99,105,115,105,111,110,70,111,114,109,97,116,0,103,108,76,105,110,107,80,114,111,103,114,97,109,0,103,108,71,101,116,80,114,111,103,114,97,109,73,110,102,111,76,111,103,0,103,108,86,97,108,105,100,97,116,101,80,114,111,103,114,97,109,0,103,108,73,115,80,114,111,103,114,97,109,0,103,108,66,105,110,100,65,116,116,114,105,98,76,111,99,97,116,105,111,110,0,103,108,66,105,110,100,70,114,97,109,101,98,117,102,102,101,114,0,103,108,71,101,110,70,114,97,109,101,98,117,102,102,101,114,115,0,103,108,68,101,108,101,116,101,70,114,97,109,101,98,117,102,102,101,114,115,0,103,108,70,114,97,109,101,98,117,102,102,101,114,82,101,110,100,101,114,98,117,102,102,101,114,0,103,108,70,114,97,109,101,98,117,102,102,101,114,84,101,120,116,117,114,101,50,68,0,103,108,71,101,116,70,114,97,109,101,98,117,102,102,101,114,65,116,116,97,99,104,109,101,110,116,80,97,114,97,109,101,116,101,114,105,118,0,103,108,73,115,70,114,97,109,101,98,117,102,102,101,114,0,103,108,68,101,108,101,116,101,79,98,106,101,99,116,0,103,108,71,101,116,79,98,106,101,99,116,80,97,114,97,109,101,116,101,114,105,118,0,103,108,71,101,116,73,110,102,111,76,111,103,0,103,108,66,105,110,100,80,114,111,103,114,97,109,0,103,108,71,101,116,80,111,105,110,116,101,114,118,0,103,108,68,114,97,119,82,97,110,103,101,69,108,101,109,101,110,116,115,0,103,108,69,110,97,98,108,101,67,108,105,101,110,116,83,116,97,116,101,0,103,108,86,101,114,116,101,120,80,111,105,110,116,101,114,0,103,108,84,101,120,67,111,111,114,100,80,111,105,110,116,101,114,0,103,108,78,111,114,109,97,108,80,111,105,110,116,101,114,0,103,108,67,111,108,111,114,80,111,105,110,116,101,114,0,103,108,67,108,105,101,110,116,65,99,116,105,118,101,84,101,120], "i8", ALLOC_NONE, Runtime.GLOBAL_BASE);
/* memory initializer */ allocate([116,117,114,101,0,103,108,71,101,110,86,101,114,116,101,120,65,114,114,97,121,115,0,103,108,68,101,108,101,116,101,86,101,114,116,101,120,65,114,114,97,121,115,0,103,108,66,105,110,100,86,101,114,116,101,120,65,114,114,97,121,0,103,108,77,97,116,114,105,120,77,111,100,101,0,103,108,76,111,97,100,73,100,101,110,116,105,116,121,0,103,108,76,111,97,100,77,97,116,114,105,120,102,0,103,108,70,114,117,115,116,117,109,0,103,108,82,111,116,97,116,101,102,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,80,111,105,110,116,101,114,0,103,108,69,110,97,98,108,101,86,101,114,116,101,120,65,116,116,114,105,98,65,114,114,97,121,0,103,108,68,105,115,97,98,108,101,86,101,114,116,101,120,65,116,116,114,105,98,65,114,114,97,121,0,103,108,68,114,97,119,65,114,114,97,121,115,0,103,108,68,114,97,119,69,108,101,109,101,110,116,115,0,103,108,83,104,97,100,101,114,66,105,110,97,114,121,0,103,108,82,101,108,101,97,115,101,83,104,97,100,101,114,67,111,109,112,105,108,101,114,0,103,108,71,101,116,69,114,114,111,114,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,68,105,118,105,115,111,114,0,103,108,68,114,97,119,65,114,114,97,121,115,73,110,115,116,97,110,99,101,100,0,103,108,68,114,97,119,69,108,101,109,101,110,116,115,73,110,115,116,97,110,99,101,100,0,103,108,70,105,110,105,115,104,0,103,108,70,108,117,115,104,0,103,108,67,108,101,97,114,68,101,112,116,104,0,103,108,67,108,101,97,114,68,101,112,116,104,102,0,103,108,68,101,112,116,104,70,117,110,99,0,103,108,69,110,97,98,108,101,0,103,108,68,105,115,97,98,108,101,0,103,108,70,114,111,110,116,70,97,99,101,0,103,108,67,117,108,108,70,97,99,101,0,103,108,67,108,101,97,114,0,103,108,76,105,110,101,87,105,100,116,104,0,103,108,67,108,101,97,114,83,116,101,110,99,105,108,0,103,108,68,101,112,116,104,77,97,115,107,0,103,108,83,116,101,110,99,105,108,77,97,115,107,0,103,108,67,104,101,99,107,70,114,97,109,101,98,117,102,102,101,114,83,116,97,116,117,115,0,103,108,71,101,110,101,114,97,116,101,77,105,112,109,97,112,0,103,108,65,99,116,105,118,101,84,101,120,116,117,114,101,0,103,108,66,108,101,110,100,69,113,117,97,116,105,111,110,0,103,108,73,115,69,110,97,98,108,101,100,0,103,108,66,108,101,110,100,70,117,110,99,0,103,108,66,108,101,110,100,69,113,117,97,116,105,111,110,83,101,112,97,114,97,116,101,0,103,108,68,101,112,116,104,82,97,110,103,101,0,103,108,68,101,112,116,104,82,97,110,103,101,102,0,103,108,83,116,101,110,99,105,108,77,97,115,107,83,101,112,97,114,97,116,101,0,103,108,72,105,110,116,0,103,108,80,111,108,121,103,111,110,79,102,102,115,101,116,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,49,102,0,103,108,83,97,109,112,108,101,67,111,118,101,114,97,103,101,0,103,108,84,101,120,80,97,114,97,109,101,116,101,114,105,0,103,108,84,101,120,80,97,114,97,109,101,116,101,114,102,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,50,102,0,103,108,83,116,101,110,99,105,108,70,117,110,99,0,103,108,83,116,101,110,99,105,108,79,112,0,103,108,86,105,101,119,112,111,114,116,0,103,108,67,108,101,97,114,67,111,108,111,114,0,103,108,83,99,105,115,115,111,114,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,51,102,0,103,108,67,111,108,111,114,77,97,115,107,0,103,108,82,101,110,100,101,114,98,117,102,102,101,114,83,116,111,114,97,103,101,0,103,108,66,108,101,110,100,70,117,110,99,83,101,112,97,114,97,116,101,0,103,108,66,108,101,110,100,67,111,108,111,114,0,103,108,83,116,101,110,99,105,108,70,117,110,99,83,101,112,97,114,97,116,101,0,103,108,83,116,101,110,99,105,108,79,112,83,101,112,97,114,97,116,101,0,103,108,86,101,114,116,101,120,65,116,116,114,105,98,52,102,0,103,108,67,111,112,121,84,101,120,73,109,97,103,101,50,68,0,103,108,67,111,112,121,84,101,120,83,117,98,73,109,97,103,101,50,68,0,103,108,68,114,97,119,66,117,102,102,101,114,115,0,123,32,77,111,100,117,108,101,46,112,114,105,110,116,69,114,114,40,39,98,97,100,32,110,97,109,101,32,105,110,32,103,101,116,80,114,111,99,65,100,100,114,101,115,115,58,32,39,32,43,32,91,80,111,105,110,116,101,114,95,115,116,114,105,110,103,105,102,121,40,36,48,41,44,32,80,111,105,110,116,101,114,95,115,116,114,105,110,103,105,102,121,40,36,49,41,93,41,59,32,125,0,17,0,10,0,17,17,17,0,0,0,0,5,0,0,0,0,0,0,9,0,0,0,0,11,0,0,0,0,0,0,0,0,17,0,15,10,17,17,17,3,10,7,0,1,19,9,11,11,0,0,9,6,11,0,0,11,0,6,17,0,0,0,17,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,17,0,10,10,17,17,17,0,10,0,0,2,0,9,11,0,0,0,9,0,11,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,12,0,0,0,0,9,12,0,0,0,0,0,12,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,13,0,0,0,4,13,0,0,0,0,9,14,0,0,0,0,0,14,0,0,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,15,0,0,0,0,15,0,0,0,0,9,16,0,0,0,0,0,16,0,0,16,0,0,18,0,0,0,18,18,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,0,0,0,18,18,18,0,0,0,0,0,0,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,10,0,0,0,0,10,0,0,0,0,9,11,0,0,0,0,0,11,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,12,0,0,0,0,9,12,0,0,0,0,0,12,0,0,12,0,0,45,43,32,32,32,48,88,48,120,0,40,110,117,108,108,41,0,45,48,88,43,48,88,32,48,88,45,48,120,43,48,120,32,48,120,0,105,110,102,0,73,78,70,0,110,97,110,0,78,65,78,0,48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70,46,0,84,33,34,25,13,1,2,3,17,75,28,12,16,4,11,29,18,30,39,104,110,111,112,113,98,32,5,6,15,19,20,21,26,8,22,7,40,36,23,24,9,10,14,27,31,37,35,131,130,125,38,42,43,60,61,62,63,67,71,74,77,88,89,90,91,92,93,94,95,96,97,99,100,101,102,103,105,106,107,108,114,115,116,121,122,123,124,0,73,108,108,101,103,97,108,32,98,121,116,101,32,115,101,113,117,101,110,99,101,0,68,111,109,97,105,110,32,101,114,114,111,114,0,82,101,115,117,108,116,32,110,111,116,32,114,101,112,114,101,115,101,110,116,97,98,108,101,0,78,111,116,32,97,32,116,116,121,0,80,101,114,109,105,115,115,105,111,110,32,100,101,110,105,101,100,0,79,112,101,114,97,116,105,111,110,32,110,111,116,32,112,101,114,109,105,116,116,101,100,0,78,111,32,115,117,99,104,32,102,105,108,101,32,111,114,32,100,105,114,101,99,116,111,114,121,0,78,111,32,115,117,99,104,32,112,114,111,99,101,115,115,0,70,105,108,101,32,101,120,105,115,116,115,0,86,97,108,117,101,32,116,111,111,32,108,97,114,103,101,32,102,111,114,32,100,97,116,97,32,116,121,112,101,0,78,111,32,115,112,97,99,101,32,108,101,102,116,32,111,110,32,100,101,118,105,99,101,0,79,117,116,32,111,102,32,109,101,109,111,114,121,0,82,101,115,111,117,114,99,101,32,98,117,115,121,0,73,110,116,101,114,114,117,112,116,101,100,32,115,121,115,116,101,109,32,99,97,108,108,0,82,101,115,111,117,114,99,101,32,116,101,109,112,111,114,97,114,105,108,121,32,117,110,97,118,97,105,108,97,98,108,101,0,73,110,118,97,108,105,100,32,115,101,101,107,0,67,114,111,115,115,45,100,101,118,105,99,101,32,108,105,110,107,0,82,101,97,100,45,111,110,108,121,32,102,105,108,101,32,115,121,115,116,101,109,0,68,105,114,101,99,116,111,114,121,32,110,111,116,32,101,109,112,116,121,0,67,111,110,110,101,99,116,105,111,110,32,114,101,115,101,116,32,98,121,32,112,101,101,114,0,79,112,101,114,97,116,105,111,110,32,116,105,109,101,100,32,111,117,116,0,67,111,110,110,101,99,116,105,111,110,32,114,101,102,117,115,101,100,0,72,111,115,116,32,105,115,32,100,111,119,110,0,72,111,115,116,32,105,115,32,117,110,114,101,97,99,104,97,98,108,101,0,65,100,100,114,101,115,115,32,105,110,32,117,115,101,0,66,114,111,107,101,110,32,112,105,112,101,0,73,47,79,32,101,114,114,111,114,0,78,111,32,115,117,99,104,32,100,101,118,105,99,101,32,111,114,32,97,100,100,114,101,115,115,0,66,108,111,99,107,32,100,101,118,105,99,101,32,114,101,113,117,105,114,101,100,0,78,111,32,115,117,99,104,32,100,101,118,105,99,101,0,78,111,116,32,97,32,100,105,114,101,99,116,111,114,121,0,73,115,32,97,32,100,105,114,101,99,116,111,114,121,0,84,101,120,116,32,102,105,108,101,32,98,117,115,121,0,69,120,101,99,32,102,111,114,109,97,116,32,101,114,114,111,114,0,73,110,118,97,108,105,100,32,97,114,103,117,109,101,110,116,0,65,114,103,117,109,101,110,116,32,108,105,115,116,32,116,111,111,32,108,111,110,103,0,83,121,109,98,111,108,105,99,32,108,105,110,107,32,108,111,111,112,0,70,105,108,101,110,97,109,101,32,116,111,111,32,108,111,110,103,0,84,111,111,32,109,97,110,121,32,111,112,101,110,32,102,105,108,101,115,32,105,110,32,115,121,115,116,101,109,0,78,111,32,102,105,108,101,32,100,101,115,99,114,105,112,116,111,114,115,32,97,118,97,105,108,97,98,108,101,0,66,97,100,32,102,105,108,101,32,100,101,115,99,114,105,112,116,111,114,0,78,111,32,99,104,105,108,100,32,112,114,111,99,101,115,115,0,66,97,100,32,97,100,100,114,101,115,115,0,70,105,108,101,32,116,111,111,32,108,97,114,103,101,0,84,111,111,32,109,97,110,121,32,108,105,110,107,115,0,78,111,32,108,111,99,107,115,32,97,118,97,105,108,97,98,108,101,0,82,101,115,111,117,114,99,101,32,100,101,97,100,108,111,99,107,32,119,111,117,108,100,32,111,99,99,117,114,0,83,116,97,116,101,32,110,111,116,32,114,101,99,111,118,101,114,97,98,108,101,0,80,114,101,118,105,111,117,115,32,111,119,110,101,114,32,100,105,101,100,0,79,112,101,114,97,116,105,111,110,32,99,97,110,99,101,108,101,100,0,70,117,110,99,116,105,111,110,32,110,111,116,32,105,109,112,108,101,109,101,110,116,101,100,0,78,111,32,109,101,115,115,97,103,101,32,111,102,32,100,101,115,105,114,101,100,32,116,121,112,101,0,73,100,101,110,116,105,102,105,101,114,32,114,101,109,111,118,101,100,0,68,101,118,105,99,101,32,110,111,116,32,97,32,115,116,114,101,97,109,0,78,111,32,100,97,116,97,32,97,118,97,105,108,97,98,108,101,0,68,101,118,105,99,101,32,116,105,109,101,111,117,116,0,79,117,116,32,111,102,32,115,116,114,101,97,109,115,32,114,101,115,111,117,114,99,101,115,0,76,105,110,107,32,104,97,115,32,98,101,101,110,32,115,101,118,101,114,101,100,0,80,114,111,116,111,99,111,108,32,101,114,114,111,114,0,66,97,100,32,109,101,115,115,97,103,101,0,70,105,108,101,32,100,101,115,99,114,105,112,116,111,114,32,105,110,32,98,97,100,32,115,116,97,116,101,0,78,111,116,32,97,32,115,111,99,107,101,116,0,68,101,115,116,105,110,97,116,105,111,110,32,97,100,100,114,101,115,115,32,114,101,113,117,105,114,101,100,0,77,101,115,115,97,103,101,32,116,111,111,32,108,97,114,103,101,0,80,114,111,116,111,99,111,108,32,119,114,111,110,103,32,116,121,112,101,32,102,111,114,32,115,111,99,107,101,116,0,80,114,111,116,111,99,111,108,32,110,111,116,32,97,118,97,105,108,97,98,108,101,0,80,114,111,116,111,99,111,108,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,83,111,99,107,101,116,32,116,121,112,101,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,78,111,116,32,115,117,112,112,111,114,116,101,100,0,80,114,111,116,111,99,111,108,32,102,97,109,105,108,121,32,110,111,116,32,115,117,112,112,111,114,116,101,100,0,65,100,100,114,101,115,115,32,102,97,109,105,108,121,32,110,111,116,32,115,117,112,112,111,114,116,101,100,32,98,121,32,112,114,111,116,111,99,111,108,0,65,100,100,114,101,115,115,32,110,111,116,32,97,118,97,105,108,97,98,108,101,0,78,101,116,119,111,114,107,32,105,115,32,100,111,119,110,0,78,101,116,119,111,114,107,32,117,110,114,101,97,99,104,97,98,108,101,0,67,111,110,110,101,99,116,105,111,110,32,114,101,115,101,116,32,98,121,32,110,101,116,119,111,114,107,0,67,111,110,110,101,99,116,105,111,110,32,97,98,111,114,116,101,100,0,78,111,32,98,117,102,102,101,114,32,115,112,97,99,101,32,97,118,97,105,108,97,98,108,101,0,83,111,99,107,101,116,32,105,115,32,99,111,110,110,101,99,116,101,100,0,83,111,99,107,101,116,32,110,111,116,32,99,111,110,110,101,99,116,101,100,0,67,97,110,110,111,116,32,115,101,110,100,32,97,102,116,101,114,32,115,111,99,107,101,116,32,115,104,117,116,100,111,119,110,0,79,112,101,114,97,116,105,111,110,32,97,108,114,101,97,100,121,32,105,110,32,112,114,111,103,114,101,115,115,0,79,112,101,114,97,116,105,111,110,32,105,110,32,112,114,111,103,114,101,115,115,0,83,116,97,108,101,32,102,105,108,101,32,104,97,110,100,108,101,0,82,101,109,111,116,101,32,73,47,79,32,101,114,114,111,114,0,81,117,111,116,97,32,101,120,99,101,101,100,101,100,0,78,111,32,109,101,100,105,117,109,32,102,111,117,110,100,0,87,114,111,110,103,32,109,101,100,105,117,109,32,116,121,112,101,0,78,111,32,101,114,114,111,114,32,105,110,102,111,114,109,97,116,105,111,110,0,0], "i8", ALLOC_NONE, Runtime.GLOBAL_BASE+10240);





/* no memory initializer */
var tempDoublePtr = STATICTOP; STATICTOP += 16;

assert(tempDoublePtr % 8 == 0);

function copyTempFloat(ptr) { // functions, because inlining this code increases code size too much

  HEAP8[tempDoublePtr] = HEAP8[ptr];

  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];

  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];

  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];

}

function copyTempDouble(ptr) {

  HEAP8[tempDoublePtr] = HEAP8[ptr];

  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];

  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];

  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];

  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];

  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];

  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];

  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];

}

// {{PRE_LIBRARY}}


  
  var GL={counter:1,lastError:0,buffers:[],mappedBuffers:{},programs:[],framebuffers:[],renderbuffers:[],textures:[],uniforms:[],shaders:[],vaos:[],contexts:[],currentContext:null,offscreenCanvases:{},timerQueriesEXT:[],byteSizeByTypeRoot:5120,byteSizeByType:[1,1,2,2,4,4,4,2,3,4,8],programInfos:{},stringCache:{},tempFixedLengthArray:[],packAlignment:4,unpackAlignment:4,init:function () {
        GL.miniTempBuffer = new Float32Array(GL.MINI_TEMP_BUFFER_SIZE);
        for (var i = 0; i < GL.MINI_TEMP_BUFFER_SIZE; i++) {
          GL.miniTempBufferViews[i] = GL.miniTempBuffer.subarray(0, i+1);
        }
  
        // For functions such as glDrawBuffers, glInvalidateFramebuffer and glInvalidateSubFramebuffer that need to pass a short array to the WebGL API,
        // create a set of short fixed-length arrays to avoid having to generate any garbage when calling those functions.
        for (var i = 0; i < 32; i++) {
          GL.tempFixedLengthArray.push(new Array(i));
        }
      },recordError:function recordError(errorCode) {
        if (!GL.lastError) {
          GL.lastError = errorCode;
        }
      },getNewId:function (table) {
        var ret = GL.counter++;
        for (var i = table.length; i < ret; i++) {
          table[i] = null;
        }
        return ret;
      },MINI_TEMP_BUFFER_SIZE:256,miniTempBuffer:null,miniTempBufferViews:[0],getSource:function (shader, count, string, length) {
        var source = '';
        for (var i = 0; i < count; ++i) {
          var frag;
          if (length) {
            var len = HEAP32[(((length)+(i*4))>>2)];
            if (len < 0) {
              frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)]);
            } else {
              frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)], len);
            }
          } else {
            frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)]);
          }
          source += frag;
        }
        return source;
      },createContext:function (canvas, webGLContextAttributes) {
        if (typeof webGLContextAttributes['majorVersion'] === 'undefined' && typeof webGLContextAttributes['minorVersion'] === 'undefined') {
          webGLContextAttributes['majorVersion'] = 1;
          webGLContextAttributes['minorVersion'] = 0;
        }
        var ctx;
        var errorInfo = '?';
        function onContextCreationError(event) {
          errorInfo = event.statusMessage || errorInfo;
        }
        try {
          canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
          try {
            if (webGLContextAttributes['majorVersion'] == 1 && webGLContextAttributes['minorVersion'] == 0) {
              ctx = canvas.getContext("webgl", webGLContextAttributes) || canvas.getContext("experimental-webgl", webGLContextAttributes);
            } else if (webGLContextAttributes['majorVersion'] == 2 && webGLContextAttributes['minorVersion'] == 0) {
              ctx = canvas.getContext("webgl2", webGLContextAttributes) || canvas.getContext("experimental-webgl2", webGLContextAttributes);
            } else {
              throw 'Unsupported WebGL context version ' + majorVersion + '.' + minorVersion + '!'
            }
          } finally {
            canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false);
          }
          if (!ctx) throw ':(';
        } catch (e) {
          Module.print('Could not create canvas: ' + [errorInfo, e, JSON.stringify(webGLContextAttributes)]);
          return 0;
        }
        // possible GL_DEBUG entry point: ctx = wrapDebugGL(ctx);
  
        if (!ctx) return 0;
        return GL.registerContext(ctx, webGLContextAttributes);
      },registerContext:function (ctx, webGLContextAttributes) {
        var handle = GL.getNewId(GL.contexts);
        var context = {
          handle: handle,
          attributes: webGLContextAttributes,
          version: webGLContextAttributes['majorVersion'],
          GLctx: ctx
        };
  
  
        // Store the created context object so that we can access the context given a canvas without having to pass the parameters again.
        if (ctx.canvas) ctx.canvas.GLctxObject = context;
        GL.contexts[handle] = context;
        if (typeof webGLContextAttributes['enableExtensionsByDefault'] === 'undefined' || webGLContextAttributes['enableExtensionsByDefault']) {
          GL.initExtensions(context);
        }
        return handle;
      },makeContextCurrent:function (contextHandle) {
        var context = GL.contexts[contextHandle];
        if (!context) return false;
        GLctx = Module.ctx = context.GLctx; // Active WebGL context object.
        GL.currentContext = context; // Active Emscripten GL layer context object.
        return true;
      },getContext:function (contextHandle) {
        return GL.contexts[contextHandle];
      },deleteContext:function (contextHandle) {
        if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
        if (typeof JSEvents === 'object') JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas); // Release all JS event handlers on the DOM element that the GL context is associated with since the context is now deleted.
        if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined; // Make sure the canvas object no longer refers to the context object so there are no GC surprises.
        GL.contexts[contextHandle] = null;
      },initExtensions:function (context) {
        // If this function is called without a specific context object, init the extensions of the currently active context.
        if (!context) context = GL.currentContext;
  
        if (context.initExtensionsDone) return;
        context.initExtensionsDone = true;
  
        var GLctx = context.GLctx;
  
        context.maxVertexAttribs = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
  
        // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist. 
  
        if (context.version < 2) {
          // Extension available from Firefox 26 and Google Chrome 30
          var instancedArraysExt = GLctx.getExtension('ANGLE_instanced_arrays');
          if (instancedArraysExt) {
            GLctx['vertexAttribDivisor'] = function(index, divisor) { instancedArraysExt['vertexAttribDivisorANGLE'](index, divisor); };
            GLctx['drawArraysInstanced'] = function(mode, first, count, primcount) { instancedArraysExt['drawArraysInstancedANGLE'](mode, first, count, primcount); };
            GLctx['drawElementsInstanced'] = function(mode, count, type, indices, primcount) { instancedArraysExt['drawElementsInstancedANGLE'](mode, count, type, indices, primcount); };
          }
  
          // Extension available from Firefox 25 and WebKit
          var vaoExt = GLctx.getExtension('OES_vertex_array_object');
          if (vaoExt) {
            GLctx['createVertexArray'] = function() { return vaoExt['createVertexArrayOES'](); };
            GLctx['deleteVertexArray'] = function(vao) { vaoExt['deleteVertexArrayOES'](vao); };
            GLctx['bindVertexArray'] = function(vao) { vaoExt['bindVertexArrayOES'](vao); };
            GLctx['isVertexArray'] = function(vao) { return vaoExt['isVertexArrayOES'](vao); };
          }
  
          var drawBuffersExt = GLctx.getExtension('WEBGL_draw_buffers');
          if (drawBuffersExt) {
            GLctx['drawBuffers'] = function(n, bufs) { drawBuffersExt['drawBuffersWEBGL'](n, bufs); };
          }
        }
  
        GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
  
        // These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
        // should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
        // As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
        // here, as long as they don't produce a performance impact for users that might not be using those extensions.
        // E.g. debugging-related extensions should probably be off by default.
        var automaticallyEnabledExtensions = [ "OES_texture_float", "OES_texture_half_float", "OES_standard_derivatives",
                                               "OES_vertex_array_object", "WEBGL_compressed_texture_s3tc", "WEBGL_depth_texture",
                                               "OES_element_index_uint", "EXT_texture_filter_anisotropic", "ANGLE_instanced_arrays",
                                               "OES_texture_float_linear", "OES_texture_half_float_linear", "WEBGL_compressed_texture_atc",
                                               "WEBGL_compressed_texture_pvrtc", "EXT_color_buffer_half_float", "WEBGL_color_buffer_float",
                                               "EXT_frag_depth", "EXT_sRGB", "WEBGL_draw_buffers", "WEBGL_shared_resources",
                                               "EXT_shader_texture_lod", "EXT_color_buffer_float"];
  
        function shouldEnableAutomatically(extension) {
          var ret = false;
          automaticallyEnabledExtensions.forEach(function(include) {
            if (ext.indexOf(include) != -1) {
              ret = true;
            }
          });
          return ret;
        }
  
        var exts = GLctx.getSupportedExtensions();
        if (exts && exts.length > 0) {
          GLctx.getSupportedExtensions().forEach(function(ext) {
            if (automaticallyEnabledExtensions.indexOf(ext) != -1) {
              GLctx.getExtension(ext); // Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
            }
          });
        }
      },populateUniformTable:function (program) {
        var p = GL.programs[program];
        GL.programInfos[program] = {
          uniforms: {},
          maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
          maxAttributeLength: -1, // This is lazily computed and cached, computed when/if first asked, "-1" meaning not computed yet.
          maxUniformBlockNameLength: -1 // Lazily computed as well
        };
  
        var ptable = GL.programInfos[program];
        var utable = ptable.uniforms;
        // A program's uniform table maps the string name of an uniform to an integer location of that uniform.
        // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
        var numUniforms = GLctx.getProgramParameter(p, GLctx.ACTIVE_UNIFORMS);
        for (var i = 0; i < numUniforms; ++i) {
          var u = GLctx.getActiveUniform(p, i);
  
          var name = u.name;
          ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);
  
          // Strip off any trailing array specifier we might have got, e.g. "[0]".
          if (name.indexOf(']', name.length-1) !== -1) {
            var ls = name.lastIndexOf('[');
            name = name.slice(0, ls);
          }
  
          // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then 
          // only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
          // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
          var loc = GLctx.getUniformLocation(p, name);
          if (loc != null)
          {
            var id = GL.getNewId(GL.uniforms);
            utable[name] = [u.size, id];
            GL.uniforms[id] = loc;
  
            for (var j = 1; j < u.size; ++j) {
              var n = name + '['+j+']';
              loc = GLctx.getUniformLocation(p, n);
              id = GL.getNewId(GL.uniforms);
  
              GL.uniforms[id] = loc;
            }
          }
        }
      }};function _emscripten_glIsRenderbuffer(renderbuffer) {
      var rb = GL.renderbuffers[renderbuffer];
      if (!rb) return 0;
      return GLctx.isRenderbuffer(rb);
    }

  function _emscripten_glStencilMaskSeparate(x0, x1) { GLctx['stencilMaskSeparate'](x0, x1) }

  
  
  function _emscripten_get_now() { abort() }
  
  
  
  function _emscripten_set_main_loop_timing(mode, value) {
      Browser.mainLoop.timingMode = mode;
      Browser.mainLoop.timingValue = value;
  
      if (!Browser.mainLoop.func) {
        console.error('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
        return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
      }
  
      if (mode == 0 /*EM_TIMING_SETTIMEOUT*/) {
        Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setTimeout() {
          var timeUntilNextTick = Math.max(0, Browser.mainLoop.tickStartTime + value - _emscripten_get_now())|0;
          setTimeout(Browser.mainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
        };
        Browser.mainLoop.method = 'timeout';
      } else if (mode == 1 /*EM_TIMING_RAF*/) {
        Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_rAF() {
          Browser.requestAnimationFrame(Browser.mainLoop.runner);
        };
        Browser.mainLoop.method = 'rAF';
      } else if (mode == 2 /*EM_TIMING_SETIMMEDIATE*/) {
        if (!window['setImmediate']) {
          // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
          var setImmediates = [];
          var emscriptenMainLoopMessageId = 'setimmediate';
          function Browser_setImmediate_messageHandler(event) {
            if (event.source === window && event.data === emscriptenMainLoopMessageId) {
              event.stopPropagation();
              setImmediates.shift()();
            }
          }
          window.addEventListener("message", Browser_setImmediate_messageHandler, true);
          window['setImmediate'] = function Browser_emulated_setImmediate(func) {
            setImmediates.push(func);
            if (ENVIRONMENT_IS_WORKER) {
              if (Module['setImmediates'] === undefined) Module['setImmediates'] = [];
              Module['setImmediates'].push(func);
              window.postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
            } else window.postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
          }
        }
        Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setImmediate() {
          window['setImmediate'](Browser.mainLoop.runner);
        };
        Browser.mainLoop.method = 'immediate';
      }
      return 0;
    }function _emscripten_set_main_loop(func, fps, simulateInfiniteLoop, arg, noSetTiming) {
      Module['noExitRuntime'] = true;
  
      assert(!Browser.mainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');
  
      Browser.mainLoop.func = func;
      Browser.mainLoop.arg = arg;
  
      var browserIterationFunc;
      if (typeof arg !== 'undefined') {
        browserIterationFunc = function() {
          Module['dynCall_vi'](func, arg);
        };
      } else {
        browserIterationFunc = function() {
          Module['dynCall_v'](func);
        };
      }
  
      var thisMainLoopId = Browser.mainLoop.currentlyRunningMainloop;
  
      Browser.mainLoop.runner = function Browser_mainLoop_runner() {
        if (ABORT) return;
        if (Browser.mainLoop.queue.length > 0) {
          var start = Date.now();
          var blocker = Browser.mainLoop.queue.shift();
          blocker.func(blocker.arg);
          if (Browser.mainLoop.remainingBlockers) {
            var remaining = Browser.mainLoop.remainingBlockers;
            var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
            if (blocker.counted) {
              Browser.mainLoop.remainingBlockers = next;
            } else {
              // not counted, but move the progress along a tiny bit
              next = next + 0.5; // do not steal all the next one's progress
              Browser.mainLoop.remainingBlockers = (8*remaining + next)/9;
            }
          }
          console.log('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + ' ms'); //, left: ' + Browser.mainLoop.remainingBlockers);
          Browser.mainLoop.updateStatus();
          
          // catches pause/resume main loop from blocker execution
          if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) return;
          
          setTimeout(Browser.mainLoop.runner, 0);
          return;
        }
  
        // catch pauses from non-main loop sources
        if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) return;
  
        // Implement very basic swap interval control
        Browser.mainLoop.currentFrameNumber = Browser.mainLoop.currentFrameNumber + 1 | 0;
        if (Browser.mainLoop.timingMode == 1/*EM_TIMING_RAF*/ && Browser.mainLoop.timingValue > 1 && Browser.mainLoop.currentFrameNumber % Browser.mainLoop.timingValue != 0) {
          // Not the scheduled time to render this frame - skip.
          Browser.mainLoop.scheduler();
          return;
        } else if (Browser.mainLoop.timingMode == 0/*EM_TIMING_SETTIMEOUT*/) {
          Browser.mainLoop.tickStartTime = _emscripten_get_now();
        }
  
        // Signal GL rendering layer that processing of a new frame is about to start. This helps it optimize
        // VBO double-buffering and reduce GPU stalls.
  
  
        if (Browser.mainLoop.method === 'timeout' && Module.ctx) {
          Module.printErr('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
          Browser.mainLoop.method = ''; // just warn once per call to set main loop
        }
  
        Browser.mainLoop.runIter(browserIterationFunc);
  
        checkStackCookie();
  
        // catch pauses from the main loop itself
        if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) return;
  
        // Queue new audio data. This is important to be right after the main loop invocation, so that we will immediately be able
        // to queue the newest produced audio samples.
        // TODO: Consider adding pre- and post- rAF callbacks so that GL.newRenderingFrameStarted() and SDL.audio.queueNewAudioData()
        //       do not need to be hardcoded into this function, but can be more generic.
        if (typeof SDL === 'object' && SDL.audio && SDL.audio.queueNewAudioData) SDL.audio.queueNewAudioData();
  
        Browser.mainLoop.scheduler();
      }
  
      if (!noSetTiming) {
        if (fps && fps > 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 1000.0 / fps);
        else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, 1); // Do rAF by rendering each frame (no decimating)
  
        Browser.mainLoop.scheduler();
      }
  
      if (simulateInfiniteLoop) {
        throw 'SimulateInfiniteLoop';
      }
    }var Browser={mainLoop:{scheduler:null,method:"",currentlyRunningMainloop:0,func:null,arg:0,timingMode:0,timingValue:0,currentFrameNumber:0,queue:[],pause:function () {
          Browser.mainLoop.scheduler = null;
          Browser.mainLoop.currentlyRunningMainloop++; // Incrementing this signals the previous main loop that it's now become old, and it must return.
        },resume:function () {
          Browser.mainLoop.currentlyRunningMainloop++;
          var timingMode = Browser.mainLoop.timingMode;
          var timingValue = Browser.mainLoop.timingValue;
          var func = Browser.mainLoop.func;
          Browser.mainLoop.func = null;
          _emscripten_set_main_loop(func, 0, false, Browser.mainLoop.arg, true /* do not set timing and call scheduler, we will do it on the next lines */);
          _emscripten_set_main_loop_timing(timingMode, timingValue);
          Browser.mainLoop.scheduler();
        },updateStatus:function () {
          if (Module['setStatus']) {
            var message = Module['statusMessage'] || 'Please wait...';
            var remaining = Browser.mainLoop.remainingBlockers;
            var expected = Browser.mainLoop.expectedBlockers;
            if (remaining) {
              if (remaining < expected) {
                Module['setStatus'](message + ' (' + (expected - remaining) + '/' + expected + ')');
              } else {
                Module['setStatus'](message);
              }
            } else {
              Module['setStatus']('');
            }
          }
        },runIter:function (func) {
          if (ABORT) return;
          if (Module['preMainLoop']) {
            var preRet = Module['preMainLoop']();
            if (preRet === false) {
              return; // |return false| skips a frame
            }
          }
          try {
            func();
          } catch (e) {
            if (e instanceof ExitStatus) {
              return;
            } else {
              if (e && typeof e === 'object' && e.stack) Module.printErr('exception thrown: ' + [e, e.stack]);
              throw e;
            }
          }
          if (Module['postMainLoop']) Module['postMainLoop']();
        }},isFullscreen:false,pointerLock:false,moduleContextCreatedCallbacks:[],workers:[],init:function () {
        if (!Module["preloadPlugins"]) Module["preloadPlugins"] = []; // needs to exist even in workers
  
        if (Browser.initted) return;
        Browser.initted = true;
  
        try {
          new Blob();
          Browser.hasBlobConstructor = true;
        } catch(e) {
          Browser.hasBlobConstructor = false;
          console.log("warning: no blob constructor, cannot create blobs with mimetypes");
        }
        Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : (typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : (!Browser.hasBlobConstructor ? console.log("warning: no BlobBuilder") : null));
        Browser.URLObject = typeof window != "undefined" ? (window.URL ? window.URL : window.webkitURL) : undefined;
        if (!Module.noImageDecoding && typeof Browser.URLObject === 'undefined') {
          console.log("warning: Browser does not support creating object URLs. Built-in browser image decoding will not be available.");
          Module.noImageDecoding = true;
        }
  
        // Support for plugins that can process preloaded files. You can add more of these to
        // your app by creating and appending to Module.preloadPlugins.
        //
        // Each plugin is asked if it can handle a file based on the file's name. If it can,
        // it is given the file's raw data. When it is done, it calls a callback with the file's
        // (possibly modified) data. For example, a plugin might decompress a file, or it
        // might create some side data structure for use later (like an Image element, etc.).
  
        var imagePlugin = {};
        imagePlugin['canHandle'] = function imagePlugin_canHandle(name) {
          return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/i.test(name);
        };
        imagePlugin['handle'] = function imagePlugin_handle(byteArray, name, onload, onerror) {
          var b = null;
          if (Browser.hasBlobConstructor) {
            try {
              b = new Blob([byteArray], { type: Browser.getMimetype(name) });
              if (b.size !== byteArray.length) { // Safari bug #118630
                // Safari's Blob can only take an ArrayBuffer
                b = new Blob([(new Uint8Array(byteArray)).buffer], { type: Browser.getMimetype(name) });
              }
            } catch(e) {
              Runtime.warnOnce('Blob constructor present but fails: ' + e + '; falling back to blob builder');
            }
          }
          if (!b) {
            var bb = new Browser.BlobBuilder();
            bb.append((new Uint8Array(byteArray)).buffer); // we need to pass a buffer, and must copy the array to get the right data range
            b = bb.getBlob();
          }
          var url = Browser.URLObject.createObjectURL(b);
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
          var img = new Image();
          img.onload = function img_onload() {
            assert(img.complete, 'Image ' + name + ' could not be decoded');
            var canvas = document.createElement('canvas');
            canvas.width = img.width;
            canvas.height = img.height;
            var ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            Module["preloadedImages"][name] = canvas;
            Browser.URLObject.revokeObjectURL(url);
            if (onload) onload(byteArray);
          };
          img.onerror = function img_onerror(event) {
            console.log('Image ' + url + ' could not be decoded');
            if (onerror) onerror();
          };
          img.src = url;
        };
        Module['preloadPlugins'].push(imagePlugin);
  
        var audioPlugin = {};
        audioPlugin['canHandle'] = function audioPlugin_canHandle(name) {
          return !Module.noAudioDecoding && name.substr(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
        };
        audioPlugin['handle'] = function audioPlugin_handle(byteArray, name, onload, onerror) {
          var done = false;
          function finish(audio) {
            if (done) return;
            done = true;
            Module["preloadedAudios"][name] = audio;
            if (onload) onload(byteArray);
          }
          function fail() {
            if (done) return;
            done = true;
            Module["preloadedAudios"][name] = new Audio(); // empty shim
            if (onerror) onerror();
          }
          if (Browser.hasBlobConstructor) {
            try {
              var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
            } catch(e) {
              return fail();
            }
            var url = Browser.URLObject.createObjectURL(b); // XXX we never revoke this!
            assert(typeof url == 'string', 'createObjectURL must return a url as a string');
            var audio = new Audio();
            audio.addEventListener('canplaythrough', function() { finish(audio) }, false); // use addEventListener due to chromium bug 124926
            audio.onerror = function audio_onerror(event) {
              if (done) return;
              console.log('warning: browser could not fully decode audio ' + name + ', trying slower base64 approach');
              function encode64(data) {
                var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
                var PAD = '=';
                var ret = '';
                var leftchar = 0;
                var leftbits = 0;
                for (var i = 0; i < data.length; i++) {
                  leftchar = (leftchar << 8) | data[i];
                  leftbits += 8;
                  while (leftbits >= 6) {
                    var curr = (leftchar >> (leftbits-6)) & 0x3f;
                    leftbits -= 6;
                    ret += BASE[curr];
                  }
                }
                if (leftbits == 2) {
                  ret += BASE[(leftchar&3) << 4];
                  ret += PAD + PAD;
                } else if (leftbits == 4) {
                  ret += BASE[(leftchar&0xf) << 2];
                  ret += PAD;
                }
                return ret;
              }
              audio.src = 'data:audio/x-' + name.substr(-3) + ';base64,' + encode64(byteArray);
              finish(audio); // we don't wait for confirmation this worked - but it's worth trying
            };
            audio.src = url;
            // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
            Browser.safeSetTimeout(function() {
              finish(audio); // try to use it even though it is not necessarily ready to play
            }, 10000);
          } else {
            return fail();
          }
        };
        Module['preloadPlugins'].push(audioPlugin);
  
        // Canvas event setup
  
        function pointerLockChange() {
          Browser.pointerLock = document['pointerLockElement'] === Module['canvas'] ||
                                document['mozPointerLockElement'] === Module['canvas'] ||
                                document['webkitPointerLockElement'] === Module['canvas'] ||
                                document['msPointerLockElement'] === Module['canvas'];
        }
        var canvas = Module['canvas'];
        if (canvas) {
          // forced aspect ratio can be enabled by defining 'forcedAspectRatio' on Module
          // Module['forcedAspectRatio'] = 4 / 3;
          
          canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                      canvas['mozRequestPointerLock'] ||
                                      canvas['webkitRequestPointerLock'] ||
                                      canvas['msRequestPointerLock'] ||
                                      function(){};
          canvas.exitPointerLock = document['exitPointerLock'] ||
                                   document['mozExitPointerLock'] ||
                                   document['webkitExitPointerLock'] ||
                                   document['msExitPointerLock'] ||
                                   function(){}; // no-op if function does not exist
          canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
  
          document.addEventListener('pointerlockchange', pointerLockChange, false);
          document.addEventListener('mozpointerlockchange', pointerLockChange, false);
          document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
          document.addEventListener('mspointerlockchange', pointerLockChange, false);
  
          if (Module['elementPointerLock']) {
            canvas.addEventListener("click", function(ev) {
              if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
                Module['canvas'].requestPointerLock();
                ev.preventDefault();
              }
            }, false);
          }
        }
      },createContext:function (canvas, useWebGL, setInModule, webGLContextAttributes) {
        if (useWebGL && Module.ctx && canvas == Module.canvas) return Module.ctx; // no need to recreate GL context if it's already been created for this canvas.
  
        var ctx;
        var contextHandle;
        if (useWebGL) {
          // For GLES2/desktop GL compatibility, adjust a few defaults to be different to WebGL defaults, so that they align better with the desktop defaults.
          var contextAttributes = {
            antialias: false,
            alpha: false
          };
  
          if (webGLContextAttributes) {
            for (var attribute in webGLContextAttributes) {
              contextAttributes[attribute] = webGLContextAttributes[attribute];
            }
          }
  
          contextHandle = GL.createContext(canvas, contextAttributes);
          if (contextHandle) {
            ctx = GL.getContext(contextHandle).GLctx;
          }
        } else {
          ctx = canvas.getContext('2d');
        }
  
        if (!ctx) return null;
  
        if (setInModule) {
          if (!useWebGL) assert(typeof GLctx === 'undefined', 'cannot set in module if GLctx is used, but we are a non-GL context that would replace it');
  
          Module.ctx = ctx;
          if (useWebGL) GL.makeContextCurrent(contextHandle);
          Module.useWebGL = useWebGL;
          Browser.moduleContextCreatedCallbacks.forEach(function(callback) { callback() });
          Browser.init();
        }
        return ctx;
      },destroyContext:function (canvas, useWebGL, setInModule) {},fullscreenHandlersInstalled:false,lockPointer:undefined,resizeCanvas:undefined,requestFullscreen:function (lockPointer, resizeCanvas, vrDevice) {
        Browser.lockPointer = lockPointer;
        Browser.resizeCanvas = resizeCanvas;
        Browser.vrDevice = vrDevice;
        if (typeof Browser.lockPointer === 'undefined') Browser.lockPointer = true;
        if (typeof Browser.resizeCanvas === 'undefined') Browser.resizeCanvas = false;
        if (typeof Browser.vrDevice === 'undefined') Browser.vrDevice = null;
  
        var canvas = Module['canvas'];
        function fullscreenChange() {
          Browser.isFullscreen = false;
          var canvasContainer = canvas.parentNode;
          if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
               document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
               document['webkitCurrentFullScreenElement']) === canvasContainer) {
            canvas.exitFullscreen = document['exitFullscreen'] ||
                                    document['cancelFullScreen'] ||
                                    document['mozCancelFullScreen'] ||
                                    document['msExitFullscreen'] ||
                                    document['webkitCancelFullScreen'] ||
                                    function() {};
            canvas.exitFullscreen = canvas.exitFullscreen.bind(document);
            if (Browser.lockPointer) canvas.requestPointerLock();
            Browser.isFullscreen = true;
            if (Browser.resizeCanvas) Browser.setFullscreenCanvasSize();
          } else {
            
            // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
            canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
            canvasContainer.parentNode.removeChild(canvasContainer);
            
            if (Browser.resizeCanvas) Browser.setWindowedCanvasSize();
          }
          if (Module['onFullScreen']) Module['onFullScreen'](Browser.isFullscreen);
          if (Module['onFullscreen']) Module['onFullscreen'](Browser.isFullscreen);
          Browser.updateCanvasDimensions(canvas);
        }
  
        if (!Browser.fullscreenHandlersInstalled) {
          Browser.fullscreenHandlersInstalled = true;
          document.addEventListener('fullscreenchange', fullscreenChange, false);
          document.addEventListener('mozfullscreenchange', fullscreenChange, false);
          document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
          document.addEventListener('MSFullscreenChange', fullscreenChange, false);
        }
  
        // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
        var canvasContainer = document.createElement("div");
        canvas.parentNode.insertBefore(canvasContainer, canvas);
        canvasContainer.appendChild(canvas);
  
        // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
        canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
                                            canvasContainer['mozRequestFullScreen'] ||
                                            canvasContainer['msRequestFullscreen'] ||
                                           (canvasContainer['webkitRequestFullscreen'] ? function() { canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null) ||
                                           (canvasContainer['webkitRequestFullScreen'] ? function() { canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);
  
        if (vrDevice) {
          canvasContainer.requestFullscreen({ vrDisplay: vrDevice });
        } else {
          canvasContainer.requestFullscreen();
        }
      },requestFullScreen:function (lockPointer, resizeCanvas, vrDevice) {
          Module.printErr('Browser.requestFullScreen() is deprecated. Please call Browser.requestFullscreen instead.');
          Browser.requestFullScreen = function(lockPointer, resizeCanvas, vrDevice) {
            return Browser.requestFullscreen(lockPointer, resizeCanvas, vrDevice);
          }
          return Browser.requestFullscreen(lockPointer, resizeCanvas, vrDevice);
      },nextRAF:0,fakeRequestAnimationFrame:function (func) {
        // try to keep 60fps between calls to here
        var now = Date.now();
        if (Browser.nextRAF === 0) {
          Browser.nextRAF = now + 1000/60;
        } else {
          while (now + 2 >= Browser.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
            Browser.nextRAF += 1000/60;
          }
        }
        var delay = Math.max(Browser.nextRAF - now, 0);
        setTimeout(func, delay);
      },requestAnimationFrame:function requestAnimationFrame(func) {
        if (typeof window === 'undefined') { // Provide fallback to setTimeout if window is undefined (e.g. in Node.js)
          Browser.fakeRequestAnimationFrame(func);
        } else {
          if (!window.requestAnimationFrame) {
            window.requestAnimationFrame = window['requestAnimationFrame'] ||
                                           window['mozRequestAnimationFrame'] ||
                                           window['webkitRequestAnimationFrame'] ||
                                           window['msRequestAnimationFrame'] ||
                                           window['oRequestAnimationFrame'] ||
                                           Browser.fakeRequestAnimationFrame;
          }
          window.requestAnimationFrame(func);
        }
      },safeCallback:function (func) {
        return function() {
          if (!ABORT) return func.apply(null, arguments);
        };
      },allowAsyncCallbacks:true,queuedAsyncCallbacks:[],pauseAsyncCallbacks:function () {
        Browser.allowAsyncCallbacks = false;
      },resumeAsyncCallbacks:function () { // marks future callbacks as ok to execute, and synchronously runs any remaining ones right now
        Browser.allowAsyncCallbacks = true;
        if (Browser.queuedAsyncCallbacks.length > 0) {
          var callbacks = Browser.queuedAsyncCallbacks;
          Browser.queuedAsyncCallbacks = [];
          callbacks.forEach(function(func) {
            func();
          });
        }
      },safeRequestAnimationFrame:function (func) {
        return Browser.requestAnimationFrame(function() {
          if (ABORT) return;
          if (Browser.allowAsyncCallbacks) {
            func();
          } else {
            Browser.queuedAsyncCallbacks.push(func);
          }
        });
      },safeSetTimeout:function (func, timeout) {
        Module['noExitRuntime'] = true;
        return setTimeout(function() {
          if (ABORT) return;
          if (Browser.allowAsyncCallbacks) {
            func();
          } else {
            Browser.queuedAsyncCallbacks.push(func);
          }
        }, timeout);
      },safeSetInterval:function (func, timeout) {
        Module['noExitRuntime'] = true;
        return setInterval(function() {
          if (ABORT) return;
          if (Browser.allowAsyncCallbacks) {
            func();
          } // drop it on the floor otherwise, next interval will kick in
        }, timeout);
      },getMimetype:function (name) {
        return {
          'jpg': 'image/jpeg',
          'jpeg': 'image/jpeg',
          'png': 'image/png',
          'bmp': 'image/bmp',
          'ogg': 'audio/ogg',
          'wav': 'audio/wav',
          'mp3': 'audio/mpeg'
        }[name.substr(name.lastIndexOf('.')+1)];
      },getUserMedia:function (func) {
        if(!window.getUserMedia) {
          window.getUserMedia = navigator['getUserMedia'] ||
                                navigator['mozGetUserMedia'];
        }
        window.getUserMedia(func);
      },getMovementX:function (event) {
        return event['movementX'] ||
               event['mozMovementX'] ||
               event['webkitMovementX'] ||
               0;
      },getMovementY:function (event) {
        return event['movementY'] ||
               event['mozMovementY'] ||
               event['webkitMovementY'] ||
               0;
      },getMouseWheelDelta:function (event) {
        var delta = 0;
        switch (event.type) {
          case 'DOMMouseScroll': 
            delta = event.detail;
            break;
          case 'mousewheel': 
            delta = event.wheelDelta;
            break;
          case 'wheel': 
            delta = event['deltaY'];
            break;
          default:
            throw 'unrecognized mouse wheel event: ' + event.type;
        }
        return delta;
      },mouseX:0,mouseY:0,mouseMovementX:0,mouseMovementY:0,touches:{},lastTouches:{},calculateMouseEvent:function (event) { // event should be mousemove, mousedown or mouseup
        if (Browser.pointerLock) {
          // When the pointer is locked, calculate the coordinates
          // based on the movement of the mouse.
          // Workaround for Firefox bug 764498
          if (event.type != 'mousemove' &&
              ('mozMovementX' in event)) {
            Browser.mouseMovementX = Browser.mouseMovementY = 0;
          } else {
            Browser.mouseMovementX = Browser.getMovementX(event);
            Browser.mouseMovementY = Browser.getMovementY(event);
          }
          
          // check if SDL is available
          if (typeof SDL != "undefined") {
          	Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
          	Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
          } else {
          	// just add the mouse delta to the current absolut mouse position
          	// FIXME: ideally this should be clamped against the canvas size and zero
          	Browser.mouseX += Browser.mouseMovementX;
          	Browser.mouseY += Browser.mouseMovementY;
          }        
        } else {
          // Otherwise, calculate the movement based on the changes
          // in the coordinates.
          var rect = Module["canvas"].getBoundingClientRect();
          var cw = Module["canvas"].width;
          var ch = Module["canvas"].height;
  
          // Neither .scrollX or .pageXOffset are defined in a spec, but
          // we prefer .scrollX because it is currently in a spec draft.
          // (see: http://www.w3.org/TR/2013/WD-cssom-view-20131217/)
          var scrollX = ((typeof window.scrollX !== 'undefined') ? window.scrollX : window.pageXOffset);
          var scrollY = ((typeof window.scrollY !== 'undefined') ? window.scrollY : window.pageYOffset);
          // If this assert lands, it's likely because the browser doesn't support scrollX or pageXOffset
          // and we have no viable fallback.
          assert((typeof scrollX !== 'undefined') && (typeof scrollY !== 'undefined'), 'Unable to retrieve scroll position, mouse positions likely broken.');
  
          if (event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchmove') {
            var touch = event.touch;
            if (touch === undefined) {
              return; // the "touch" property is only defined in SDL
  
            }
            var adjustedX = touch.pageX - (scrollX + rect.left);
            var adjustedY = touch.pageY - (scrollY + rect.top);
  
            adjustedX = adjustedX * (cw / rect.width);
            adjustedY = adjustedY * (ch / rect.height);
  
            var coords = { x: adjustedX, y: adjustedY };
            
            if (event.type === 'touchstart') {
              Browser.lastTouches[touch.identifier] = coords;
              Browser.touches[touch.identifier] = coords;
            } else if (event.type === 'touchend' || event.type === 'touchmove') {
              var last = Browser.touches[touch.identifier];
              if (!last) last = coords;
              Browser.lastTouches[touch.identifier] = last;
              Browser.touches[touch.identifier] = coords;
            } 
            return;
          }
  
          var x = event.pageX - (scrollX + rect.left);
          var y = event.pageY - (scrollY + rect.top);
  
          // the canvas might be CSS-scaled compared to its backbuffer;
          // SDL-using content will want mouse coordinates in terms
          // of backbuffer units.
          x = x * (cw / rect.width);
          y = y * (ch / rect.height);
  
          Browser.mouseMovementX = x - Browser.mouseX;
          Browser.mouseMovementY = y - Browser.mouseY;
          Browser.mouseX = x;
          Browser.mouseY = y;
        }
      },asyncLoad:function (url, onload, onerror, noRunDep) {
        var dep = !noRunDep ? getUniqueRunDependency('al ' + url) : '';
        Module['readAsync'](url, function(arrayBuffer) {
          assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
          onload(new Uint8Array(arrayBuffer));
          if (dep) removeRunDependency(dep);
        }, function(event) {
          if (onerror) {
            onerror();
          } else {
            throw 'Loading data file "' + url + '" failed.';
          }
        });
        if (dep) addRunDependency(dep);
      },resizeListeners:[],updateResizeListeners:function () {
        var canvas = Module['canvas'];
        Browser.resizeListeners.forEach(function(listener) {
          listener(canvas.width, canvas.height);
        });
      },setCanvasSize:function (width, height, noUpdates) {
        var canvas = Module['canvas'];
        Browser.updateCanvasDimensions(canvas, width, height);
        if (!noUpdates) Browser.updateResizeListeners();
      },windowedWidth:0,windowedHeight:0,setFullscreenCanvasSize:function () {
        // check if SDL is available   
        if (typeof SDL != "undefined") {
        	var flags = HEAPU32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)];
        	flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
        	HEAP32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)]=flags
        }
        Browser.updateResizeListeners();
      },setWindowedCanvasSize:function () {
        // check if SDL is available       
        if (typeof SDL != "undefined") {
        	var flags = HEAPU32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)];
        	flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
        	HEAP32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)]=flags
        }
        Browser.updateResizeListeners();
      },updateCanvasDimensions:function (canvas, wNative, hNative) {
        if (wNative && hNative) {
          canvas.widthNative = wNative;
          canvas.heightNative = hNative;
        } else {
          wNative = canvas.widthNative;
          hNative = canvas.heightNative;
        }
        var w = wNative;
        var h = hNative;
        if (Module['forcedAspectRatio'] && Module['forcedAspectRatio'] > 0) {
          if (w/h < Module['forcedAspectRatio']) {
            w = Math.round(h * Module['forcedAspectRatio']);
          } else {
            h = Math.round(w / Module['forcedAspectRatio']);
          }
        }
        if (((document['fullscreenElement'] || document['mozFullScreenElement'] ||
             document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
             document['webkitCurrentFullScreenElement']) === canvas.parentNode) && (typeof screen != 'undefined')) {
           var factor = Math.min(screen.width / w, screen.height / h);
           w = Math.round(w * factor);
           h = Math.round(h * factor);
        }
        if (Browser.resizeCanvas) {
          if (canvas.width  != w) canvas.width  = w;
          if (canvas.height != h) canvas.height = h;
          if (typeof canvas.style != 'undefined') {
            canvas.style.removeProperty( "width");
            canvas.style.removeProperty("height");
          }
        } else {
          if (canvas.width  != wNative) canvas.width  = wNative;
          if (canvas.height != hNative) canvas.height = hNative;
          if (typeof canvas.style != 'undefined') {
            if (w != wNative || h != hNative) {
              canvas.style.setProperty( "width", w + "px", "important");
              canvas.style.setProperty("height", h + "px", "important");
            } else {
              canvas.style.removeProperty( "width");
              canvas.style.removeProperty("height");
            }
          }
        }
      },wgetRequests:{},nextWgetRequestHandle:0,getNextWgetRequestHandle:function () {
        var handle = Browser.nextWgetRequestHandle;
        Browser.nextWgetRequestHandle++;
        return handle;
      }};var GLFW={Window:function (id, width, height, title, monitor, share) {
        this.id = id;
        this.x = 0;
        this.y = 0;
        this.fullscreen = false; // Used to determine if app in fullscreen mode
        this.storedX = 0; // Used to store X before fullscreen
        this.storedY = 0; // Used to store Y before fullscreen
        this.width = width;
        this.height = height;
        this.storedWidth = width; // Used to store width before fullscreen
        this.storedHeight = height; // Used to store height before fullscreen
        this.title = title;
        this.monitor = monitor;
        this.share = share;
        this.attributes = GLFW.hints;
        this.inputModes = {
          0x00033001:0x00034001, // GLFW_CURSOR (GLFW_CURSOR_NORMAL)
          0x00033002:0, // GLFW_STICKY_KEYS
          0x00033003:0, // GLFW_STICKY_MOUSE_BUTTONS
        };
        this.buttons = 0;
        this.keys = new Array();
        this.shouldClose = 0;
        this.title = null;
        this.windowPosFunc = null; // GLFWwindowposfun
        this.windowSizeFunc = null; // GLFWwindowsizefun
        this.windowCloseFunc = null; // GLFWwindowclosefun
        this.windowRefreshFunc = null; // GLFWwindowrefreshfun
        this.windowFocusFunc = null; // GLFWwindowfocusfun
        this.windowIconifyFunc = null; // GLFWwindowiconifyfun
        this.framebufferSizeFunc = null; // GLFWframebuffersizefun
        this.mouseButtonFunc = null; // GLFWmousebuttonfun
        this.cursorPosFunc = null; // GLFWcursorposfun
        this.cursorEnterFunc = null; // GLFWcursorenterfun
        this.scrollFunc = null; // GLFWscrollfun
        this.keyFunc = null; // GLFWkeyfun
        this.charFunc = null; // GLFWcharfun
        this.userptr = null;
      },WindowFromId:function (id) {
        if (id <= 0 || !GLFW.windows) return null;
        return GLFW.windows[id - 1];
      },errorFunc:null,monitorFunc:null,active:null,windows:null,monitors:null,monitorString:null,versionString:null,initialTime:null,extensions:null,hints:null,defaultHints:{131073:0,131074:0,131075:1,131076:1,131077:1,135169:8,135170:8,135171:8,135172:8,135173:24,135174:8,135175:0,135176:0,135177:0,135178:0,135179:0,135180:0,135181:0,135182:0,135183:0,139265:196609,139266:1,139267:0,139268:0,139269:0,139270:0,139271:0,139272:0},DOMToGLFWKeyCode:function (keycode) {
        switch (keycode) {
          // these keycodes are only defined for GLFW3, assume they are the same for GLFW2
          case 0x20:return 32; // DOM_VK_SPACE -> GLFW_KEY_SPACE
          case 0xDE:return 39; // DOM_VK_QUOTE -> GLFW_KEY_APOSTROPHE
          case 0xBC:return 44; // DOM_VK_COMMA -> GLFW_KEY_COMMA
          case 0xAD:return 45; // DOM_VK_HYPHEN_MINUS -> GLFW_KEY_MINUS
          case 0xBD:return 45; // DOM_VK_MINUS -> GLFW_KEY_MINUS
          case 0xBE:return 46; // DOM_VK_PERIOD -> GLFW_KEY_PERIOD
          case 0xBF:return 47; // DOM_VK_SLASH -> GLFW_KEY_SLASH
          case 0x30:return 48; // DOM_VK_0 -> GLFW_KEY_0
          case 0x31:return 49; // DOM_VK_1 -> GLFW_KEY_1
          case 0x32:return 50; // DOM_VK_2 -> GLFW_KEY_2
          case 0x33:return 51; // DOM_VK_3 -> GLFW_KEY_3
          case 0x34:return 52; // DOM_VK_4 -> GLFW_KEY_4
          case 0x35:return 53; // DOM_VK_5 -> GLFW_KEY_5
          case 0x36:return 54; // DOM_VK_6 -> GLFW_KEY_6
          case 0x37:return 55; // DOM_VK_7 -> GLFW_KEY_7
          case 0x38:return 56; // DOM_VK_8 -> GLFW_KEY_8
          case 0x39:return 57; // DOM_VK_9 -> GLFW_KEY_9
          case 0x3B:return 59; // DOM_VK_SEMICOLON -> GLFW_KEY_SEMICOLON
          case 0x3D:return 61; // DOM_VK_EQUALS -> GLFW_KEY_EQUAL
          case 0xBB:return 61; // DOM_VK_EQUALS -> GLFW_KEY_EQUAL
          case 0x41:return 65; // DOM_VK_A -> GLFW_KEY_A
          case 0x42:return 66; // DOM_VK_B -> GLFW_KEY_B
          case 0x43:return 67; // DOM_VK_C -> GLFW_KEY_C
          case 0x44:return 68; // DOM_VK_D -> GLFW_KEY_D
          case 0x45:return 69; // DOM_VK_E -> GLFW_KEY_E
          case 0x46:return 70; // DOM_VK_F -> GLFW_KEY_F
          case 0x47:return 71; // DOM_VK_G -> GLFW_KEY_G
          case 0x48:return 72; // DOM_VK_H -> GLFW_KEY_H
          case 0x49:return 73; // DOM_VK_I -> GLFW_KEY_I
          case 0x4A:return 74; // DOM_VK_J -> GLFW_KEY_J
          case 0x4B:return 75; // DOM_VK_K -> GLFW_KEY_K
          case 0x4C:return 76; // DOM_VK_L -> GLFW_KEY_L
          case 0x4D:return 77; // DOM_VK_M -> GLFW_KEY_M
          case 0x4E:return 78; // DOM_VK_N -> GLFW_KEY_N
          case 0x4F:return 79; // DOM_VK_O -> GLFW_KEY_O
          case 0x50:return 80; // DOM_VK_P -> GLFW_KEY_P
          case 0x51:return 81; // DOM_VK_Q -> GLFW_KEY_Q
          case 0x52:return 82; // DOM_VK_R -> GLFW_KEY_R
          case 0x53:return 83; // DOM_VK_S -> GLFW_KEY_S
          case 0x54:return 84; // DOM_VK_T -> GLFW_KEY_T
          case 0x55:return 85; // DOM_VK_U -> GLFW_KEY_U
          case 0x56:return 86; // DOM_VK_V -> GLFW_KEY_V
          case 0x57:return 87; // DOM_VK_W -> GLFW_KEY_W
          case 0x58:return 88; // DOM_VK_X -> GLFW_KEY_X
          case 0x59:return 89; // DOM_VK_Y -> GLFW_KEY_Y
          case 0x5a:return 90; // DOM_VK_Z -> GLFW_KEY_Z
          case 0xDB:return 91; // DOM_VK_OPEN_BRACKET -> GLFW_KEY_LEFT_BRACKET
          case 0xDC:return 92; // DOM_VK_BACKSLASH -> GLFW_KEY_BACKSLASH
          case 0xDD:return 93; // DOM_VK_CLOSE_BRACKET -> GLFW_KEY_RIGHT_BRACKET
          case 0xC0:return 94; // DOM_VK_BACK_QUOTE -> GLFW_KEY_GRAVE_ACCENT
          
  
          case 0x1B:return 256; // DOM_VK_ESCAPE -> GLFW_KEY_ESCAPE
          case 0x0D:return 257; // DOM_VK_RETURN -> GLFW_KEY_ENTER
          case 0x09:return 258; // DOM_VK_TAB -> GLFW_KEY_TAB
          case 0x08:return 259; // DOM_VK_BACK -> GLFW_KEY_BACKSPACE
          case 0x2D:return 260; // DOM_VK_INSERT -> GLFW_KEY_INSERT
          case 0x2E:return 261; // DOM_VK_DELETE -> GLFW_KEY_DELETE
          case 0x27:return 262; // DOM_VK_RIGHT -> GLFW_KEY_RIGHT
          case 0x25:return 263; // DOM_VK_LEFT -> GLFW_KEY_LEFT
          case 0x28:return 264; // DOM_VK_DOWN -> GLFW_KEY_DOWN
          case 0x26:return 265; // DOM_VK_UP -> GLFW_KEY_UP
          case 0x21:return 266; // DOM_VK_PAGE_UP -> GLFW_KEY_PAGE_UP
          case 0x22:return 267; // DOM_VK_PAGE_DOWN -> GLFW_KEY_PAGE_DOWN
          case 0x24:return 268; // DOM_VK_HOME -> GLFW_KEY_HOME
          case 0x23:return 269; // DOM_VK_END -> GLFW_KEY_END
          case 0x14:return 280; // DOM_VK_CAPS_LOCK -> GLFW_KEY_CAPS_LOCK
          case 0x91:return 281; // DOM_VK_SCROLL_LOCK -> GLFW_KEY_SCROLL_LOCK
          case 0x90:return 282; // DOM_VK_NUM_LOCK -> GLFW_KEY_NUM_LOCK
          case 0x2C:return 283; // DOM_VK_SNAPSHOT -> GLFW_KEY_PRINT_SCREEN
          case 0x13:return 284; // DOM_VK_PAUSE -> GLFW_KEY_PAUSE
          case 0x70:return 290; // DOM_VK_F1 -> GLFW_KEY_F1
          case 0x71:return 291; // DOM_VK_F2 -> GLFW_KEY_F2
          case 0x72:return 292; // DOM_VK_F3 -> GLFW_KEY_F3
          case 0x73:return 293; // DOM_VK_F4 -> GLFW_KEY_F4
          case 0x74:return 294; // DOM_VK_F5 -> GLFW_KEY_F5
          case 0x75:return 295; // DOM_VK_F6 -> GLFW_KEY_F6
          case 0x76:return 296; // DOM_VK_F7 -> GLFW_KEY_F7
          case 0x77:return 297; // DOM_VK_F8 -> GLFW_KEY_F8
          case 0x78:return 298; // DOM_VK_F9 -> GLFW_KEY_F9
          case 0x79:return 299; // DOM_VK_F10 -> GLFW_KEY_F10
          case 0x7A:return 300; // DOM_VK_F11 -> GLFW_KEY_F11
          case 0x7B:return 301; // DOM_VK_F12 -> GLFW_KEY_F12
          case 0x7C:return 302; // DOM_VK_F13 -> GLFW_KEY_F13
          case 0x7D:return 303; // DOM_VK_F14 -> GLFW_KEY_F14
          case 0x7E:return 304; // DOM_VK_F15 -> GLFW_KEY_F15
          case 0x7F:return 305; // DOM_VK_F16 -> GLFW_KEY_F16
          case 0x80:return 306; // DOM_VK_F17 -> GLFW_KEY_F17
          case 0x81:return 307; // DOM_VK_F18 -> GLFW_KEY_F18
          case 0x82:return 308; // DOM_VK_F19 -> GLFW_KEY_F19
          case 0x83:return 309; // DOM_VK_F20 -> GLFW_KEY_F20
          case 0x84:return 310; // DOM_VK_F21 -> GLFW_KEY_F21
          case 0x85:return 311; // DOM_VK_F22 -> GLFW_KEY_F22
          case 0x86:return 312; // DOM_VK_F23 -> GLFW_KEY_F23
          case 0x87:return 313; // DOM_VK_F24 -> GLFW_KEY_F24
          case 0x88:return 314; // 0x88 (not used?) -> GLFW_KEY_F25
          case 0x60:return 320; // DOM_VK_NUMPAD0 -> GLFW_KEY_KP_0
          case 0x61:return 321; // DOM_VK_NUMPAD1 -> GLFW_KEY_KP_1
          case 0x62:return 322; // DOM_VK_NUMPAD2 -> GLFW_KEY_KP_2
          case 0x63:return 323; // DOM_VK_NUMPAD3 -> GLFW_KEY_KP_3
          case 0x64:return 324; // DOM_VK_NUMPAD4 -> GLFW_KEY_KP_4
          case 0x65:return 325; // DOM_VK_NUMPAD5 -> GLFW_KEY_KP_5
          case 0x66:return 326; // DOM_VK_NUMPAD6 -> GLFW_KEY_KP_6
          case 0x67:return 327; // DOM_VK_NUMPAD7 -> GLFW_KEY_KP_7
          case 0x68:return 328; // DOM_VK_NUMPAD8 -> GLFW_KEY_KP_8
          case 0x69:return 329; // DOM_VK_NUMPAD9 -> GLFW_KEY_KP_9
          case 0x6E:return 330; // DOM_VK_DECIMAL -> GLFW_KEY_KP_DECIMAL
          case 0x6F:return 331; // DOM_VK_DIVIDE -> GLFW_KEY_KP_DIVIDE
          case 0x6A:return 332; // DOM_VK_MULTIPLY -> GLFW_KEY_KP_MULTIPLY
          case 0x6D:return 333; // DOM_VK_SUBTRACT -> GLFW_KEY_KP_SUBTRACT
          case 0x6B:return 334; // DOM_VK_ADD -> GLFW_KEY_KP_ADD
          // case 0x0D:return 335; // DOM_VK_RETURN -> GLFW_KEY_KP_ENTER (DOM_KEY_LOCATION_RIGHT)
          // case 0x61:return 336; // DOM_VK_EQUALS -> GLFW_KEY_KP_EQUAL (DOM_KEY_LOCATION_RIGHT)
          case 0x10:return 340; // DOM_VK_SHIFT -> GLFW_KEY_LEFT_SHIFT
          case 0x11:return 341; // DOM_VK_CONTROL -> GLFW_KEY_LEFT_CONTROL
          case 0x12:return 342; // DOM_VK_ALT -> GLFW_KEY_LEFT_ALT
          case 0x5B:return 343; // DOM_VK_WIN -> GLFW_KEY_LEFT_SUPER
          // case 0x10:return 344; // DOM_VK_SHIFT -> GLFW_KEY_RIGHT_SHIFT (DOM_KEY_LOCATION_RIGHT)
          // case 0x11:return 345; // DOM_VK_CONTROL -> GLFW_KEY_RIGHT_CONTROL (DOM_KEY_LOCATION_RIGHT)
          // case 0x12:return 346; // DOM_VK_ALT -> GLFW_KEY_RIGHT_ALT (DOM_KEY_LOCATION_RIGHT)
          // case 0x5B:return 347; // DOM_VK_WIN -> GLFW_KEY_RIGHT_SUPER (DOM_KEY_LOCATION_RIGHT)
          case 0x5D:return 348; // DOM_VK_CONTEXT_MENU -> GLFW_KEY_MENU
          // XXX: GLFW_KEY_WORLD_1, GLFW_KEY_WORLD_2 what are these?
          default:return -1; // GLFW_KEY_UNKNOWN
        };
      },getModBits:function (win) {
        var mod = 0;
        if (win.keys[340]) mod |= 0x0001; // GLFW_MOD_SHIFT
        if (win.keys[341]) mod |= 0x0002; // GLFW_MOD_CONTROL
        if (win.keys[342]) mod |= 0x0004; // GLFW_MOD_ALT
        if (win.keys[343]) mod |= 0x0008; // GLFW_MOD_SUPER
        return mod;
      },onKeyPress:function (event) {
        if (!GLFW.active || !GLFW.active.charFunc) return;
  
        // correct unicode charCode is only available with onKeyPress event
        var charCode = event.charCode;
        if (charCode == 0 || (charCode >= 0x00 && charCode <= 0x1F)) return;
  
  
        Module['dynCall_vii'](GLFW.active.charFunc, GLFW.active.id, charCode);
      },onKeyChanged:function (event, status) {
        if (!GLFW.active) return;
  
        var key = GLFW.DOMToGLFWKeyCode(event.keyCode);
        if (key == -1) return;
  
        var repeat = status && GLFW.active.keys[key];
        GLFW.active.keys[key] = status;
        if (!GLFW.active.keyFunc) return;
  
  
        if (repeat) status = 2; // GLFW_REPEAT
        Module['dynCall_viiiii'](GLFW.active.keyFunc, GLFW.active.id, key, event.keyCode, status, GLFW.getModBits(GLFW.active));
      },onKeydown:function (event) {
        GLFW.onKeyChanged(event, 1); // GLFW_PRESS or GLFW_REPEAT
  
        // This logic comes directly from the sdl implementation. We cannot
        // call preventDefault on all keydown events otherwise onKeyPress will
        // not get called
        if (event.keyCode === 8 /* backspace */ || event.keyCode === 9 /* tab */) {
          event.preventDefault();
        }
      },onKeyup:function (event) {
        GLFW.onKeyChanged(event, 0); // GLFW_RELEASE
      },onMousemove:function (event) {
        if (!GLFW.active) return;
  
        Browser.calculateMouseEvent(event);
  
        if (event.target != Module["canvas"] || !GLFW.active.cursorPosFunc) return;
  
  
        Module['dynCall_vidd'](GLFW.active.cursorPosFunc, GLFW.active.id, Browser.mouseX, Browser.mouseY);
      },DOMToGLFWMouseButton:function (event) {
        // DOM and glfw have different button codes.
        // See http://www.w3schools.com/jsref/event_button.asp.
        var eventButton = event['button'];
        if (eventButton > 0) {
          if (eventButton == 1) {
            eventButton = 2;
          } else {
            eventButton = 1;
          }
        }
        return eventButton;
      },onMouseenter:function (event) {
        if (!GLFW.active) return;
  
        if (event.target != Module["canvas"] || !GLFW.active.cursorEnterFunc) return;
  
        Module['dynCall_vii'](GLFW.active.cursorEnterFunc, GLFW.active.id, 1);
      },onMouseleave:function (event) {
        if (!GLFW.active) return;
  
        if (event.target != Module["canvas"] || !GLFW.active.cursorEnterFunc) return;
  
        Module['dynCall_vii'](GLFW.active.cursorEnterFunc, GLFW.active.id, 0);
      },onMouseButtonChanged:function (event, status) {
        if (!GLFW.active) return;
  
        Browser.calculateMouseEvent(event);
  
        if (event.target != Module["canvas"]) return;
  
        eventButton = GLFW.DOMToGLFWMouseButton(event);
  
        if (status == 1) { // GLFW_PRESS
          GLFW.active.buttons |= (1 << eventButton);
          try {
            event.target.setCapture();
          } catch (e) {}
        } else {  // GLFW_RELEASE
          GLFW.active.buttons &= ~(1 << eventButton);
        }
  
        if (!GLFW.active.mouseButtonFunc) return;
  
  
        Module['dynCall_viiii'](GLFW.active.mouseButtonFunc, GLFW.active.id, eventButton, status, GLFW.getModBits(GLFW.active));
      },onMouseButtonDown:function (event) {
        if (!GLFW.active) return;
        GLFW.onMouseButtonChanged(event, 1); // GLFW_PRESS
      },onMouseButtonUp:function (event) {
        if (!GLFW.active) return;
        GLFW.onMouseButtonChanged(event, 0); // GLFW_RELEASE
      },onMouseWheel:function (event) {
        // Note the minus sign that flips browser wheel direction (positive direction scrolls page down) to native wheel direction (positive direction is mouse wheel up)
        var delta = -Browser.getMouseWheelDelta(event);
        delta = (delta == 0) ? 0 : (delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1)); // Quantize to integer so that minimum scroll is at least +/- 1.
        GLFW.wheelPos += delta;
  
        if (!GLFW.active || !GLFW.active.scrollFunc || event.target != Module['canvas']) return;
  
  
        var sx = 0;
        var sy = 0;
        if (event.type == 'mousewheel') {
          sx = event.wheelDeltaX;
          sy = event.wheelDeltaY;
        } else {
          sx = event.deltaX;
          sy = event.deltaY;
        }
  
        Module['dynCall_vidd'](GLFW.active.scrollFunc, GLFW.active.id, sx, sy);
  
        event.preventDefault();
      },onCanvasResize:function (width, height) {
        if (!GLFW.active) return;
  
        var resizeNeeded = true;
  
        // If the client is requestiong fullscreen mode
        if (document["fullscreen"] || document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
          GLFW.active.storedX = GLFW.active.x;
          GLFW.active.storedY = GLFW.active.y;
          GLFW.active.storedWidth = GLFW.active.width;
          GLFW.active.storedHeight = GLFW.active.height;
          GLFW.active.x = GLFW.active.y = 0;
          GLFW.active.width = screen.width;
          GLFW.active.height = screen.height;
          GLFW.active.fullscreen = true;
  
        // If the client is reverting from fullscreen mode
        } else if (GLFW.active.fullscreen == true) {
          GLFW.active.x = GLFW.active.storedX;
          GLFW.active.y = GLFW.active.storedY;
          GLFW.active.width = GLFW.active.storedWidth;
          GLFW.active.height = GLFW.active.storedHeight;
          GLFW.active.fullscreen = false;
  
        // If the width/height values do not match current active window sizes
        } else if (GLFW.active.width != width || GLFW.active.height != height) {
            GLFW.active.width = width;
            GLFW.active.height = height;
        } else {
          resizeNeeded = false;
        }
  
        // If any of the above conditions were true, we need to resize the canvas
        if (resizeNeeded) {
          // resets the canvas size to counter the aspect preservation of Browser.updateCanvasDimensions
          Browser.setCanvasSize(GLFW.active.width, GLFW.active.height, true);
          // TODO: Client dimensions (clientWidth/clientHeight) vs pixel dimensions (width/height) of
          // the canvas should drive window and framebuffer size respectfully.
          GLFW.onWindowSizeChanged();
          GLFW.onFramebufferSizeChanged();
        }
      },onWindowSizeChanged:function () {
        if (!GLFW.active) return;
  
        if (!GLFW.active.windowSizeFunc) return;
  
  
        Module['dynCall_viii'](GLFW.active.windowSizeFunc, GLFW.active.id, GLFW.active.width, GLFW.active.height);
      },onFramebufferSizeChanged:function () {
        if (!GLFW.active) return;
  
        if (!GLFW.active.framebufferSizeFunc) return;
  
        Module['dynCall_viii'](GLFW.active.framebufferSizeFunc, GLFW.active.id, GLFW.active.width, GLFW.active.height);
      },requestFullscreen:function () {
        var RFS = Module["canvas"]['requestFullscreen'] ||
                  Module["canvas"]['mozRequestFullScreen'] ||
                  Module["canvas"]['webkitRequestFullScreen'] ||
                  (function() {});
        RFS.apply(Module["canvas"], []);
      },requestFullScreen:function () {
        Module.printErr('GLFW.requestFullScreen() is deprecated. Please call GLFW.requestFullscreen instead.');
        GLFW.requestFullScreen = function() {
          return GLFW.requestFullscreen();
        }
        return GLFW.requestFullscreen();
      },exitFullscreen:function () {
        var CFS = document['exitFullscreen'] ||
                  document['cancelFullScreen'] ||
                  document['mozCancelFullScreen'] ||
                  document['webkitCancelFullScreen'] ||
            (function() {});
        CFS.apply(document, []);
      },cancelFullScreen:function () {
        Module.printErr('GLFW.cancelFullScreen() is deprecated. Please call GLFW.exitFullscreen instead.');
        GLFW.cancelFullScreen = function() {
          return GLFW.exitFullscreen();
        }
        return GLFW.exitFullscreen();
      },getTime:function () {
        return _emscripten_get_now() / 1000;
      },setWindowTitle:function (winid, title) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
  
        win.title = Pointer_stringify(title);
        if (GLFW.active.id == win.id) {
          document.title = win.title;
        }
      },setKeyCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.keyFunc = cbfun;
      },setCharCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.charFunc = cbfun;
      },setMouseButtonCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.mouseButtonFunc = cbfun;
      },setCursorPosCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.cursorPosFunc = cbfun;
      },setScrollCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.scrollFunc = cbfun;
      },setWindowSizeCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.windowSizeFunc = cbfun;
       
      },setWindowCloseCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.windowCloseFunc = cbfun;
      },setWindowRefreshCallback:function (winid, cbfun) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.windowRefreshFunc = cbfun;
      },onClickRequestPointerLock:function (e) {
        if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
          Module['canvas'].requestPointerLock();
          e.preventDefault();
        }
      },setInputMode:function (winid, mode, value) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
  
        switch(mode) {
          case 0x00033001: { // GLFW_CURSOR
            switch(value) {
              case 0x00034001: { // GLFW_CURSOR_NORMAL
                win.inputModes[mode] = value;
                Module['canvas'].removeEventListener('click', GLFW.onClickRequestPointerLock, true);
                Module['canvas'].exitPointerLock();
                break;
              }
              case 0x00034002: { // GLFW_CURSOR_HIDDEN
                console.log("glfwSetInputMode called with GLFW_CURSOR_HIDDEN value not implemented.");
                break;
              }
              case 0x00034003: { // GLFW_CURSOR_DISABLED
                win.inputModes[mode] = value;
                Module['canvas'].addEventListener('click', GLFW.onClickRequestPointerLock, true);
                Module['canvas'].requestPointerLock();
                break;
              }
              default: {
                console.log("glfwSetInputMode called with unknown value parameter value: " + value + ".");
                break;
              }
            }
            break;
          }
          case 0x00033002: { // GLFW_STICKY_KEYS
            console.log("glfwSetInputMode called with GLFW_STICKY_KEYS mode not implemented.");
            break;
          }
          case 0x00033003: { // GLFW_STICKY_MOUSE_BUTTONS
            console.log("glfwSetInputMode called with GLFW_STICKY_MOUSE_BUTTONS mode not implemented.");
            break;
          }
          default: {
            console.log("glfwSetInputMode called with unknown mode parameter value: " + mode + ".");
            break;
          }
        }
      },getKey:function (winid, key) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return 0;
        return win.keys[key];
      },getMouseButton:function (winid, button) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return 0;
        return (win.buttons & (1 << button)) > 0;
      },getCursorPos:function (winid, x, y) {
        setValue(x, Browser.mouseX, 'double');
        setValue(y, Browser.mouseY, 'double');
      },getMousePos:function (winid, x, y) {
        setValue(x, Browser.mouseX, 'i32');
        setValue(y, Browser.mouseY, 'i32');
      },setCursorPos:function (winid, x, y) {
      },getWindowPos:function (winid, x, y) {
        var wx = 0;
        var wy = 0;
  
        var win = GLFW.WindowFromId(winid);
        if (win) {
          wx = win.x;
          wy = win.y;
        }
  
        setValue(x, wx, 'i32');
        setValue(y, wy, 'i32');
      },setWindowPos:function (winid, x, y) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
        win.x = x;
        win.y = y;
      },getWindowSize:function (winid, width, height) {
        var ww = 0;
        var wh = 0;
  
        var win = GLFW.WindowFromId(winid);
        if (win) {
          ww = win.width;
          wh = win.height;
        }
  
        setValue(width, ww, 'i32');
        setValue(height, wh, 'i32');
      },setWindowSize:function (winid, width, height) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
  
        if (GLFW.active.id == win.id) {
          if (width == screen.width && height == screen.height) {
            GLFW.requestFullscreen();
          } else {
            GLFW.exitFullscreen();
            Browser.setCanvasSize(width, height);
            win.width = width;
            win.height = height;
          }
        }
  
        if (!win.windowSizeFunc) return;
  
  
        Module['dynCall_viii'](win.windowSizeFunc, win.id, width, height);
      },createWindow:function (width, height, title, monitor, share) {
        var i, id;
        for (i = 0; i < GLFW.windows.length && GLFW.windows[i] !== null; i++);
        if (i > 0) throw "glfwCreateWindow only supports one window at time currently";
  
        // id for window
        id = i + 1;
  
        // not valid
        if (width <= 0 || height <= 0) return 0;
  
        if (monitor) {
          GLFW.requestFullscreen();
        } else {
          Browser.setCanvasSize(width, height);
        }
  
        // Create context when there are no existing alive windows
        for (i = 0; i < GLFW.windows.length && GLFW.windows[i] == null; i++);
        if (i == GLFW.windows.length) {
          var contextAttributes = {
            antialias: (GLFW.hints[0x0002100D] > 1), // GLFW_SAMPLES
            depth: (GLFW.hints[0x00021005] > 0),     // GLFW_DEPTH_BITS
            stencil: (GLFW.hints[0x00021006] > 0),   // GLFW_STENCIL_BITS
            alpha: (GLFW.hints[0x00021004] > 0)      // GLFW_ALPHA_BITS 
          }
          Module.ctx = Browser.createContext(Module['canvas'], true, true, contextAttributes);
        }
  
        // If context creation failed, do not return a valid window
        if (!Module.ctx) return 0;
  
        // Get non alive id
        var win = new GLFW.Window(id, width, height, title, monitor, share);
  
        // Set window to array
        if (id - 1 == GLFW.windows.length) {
          GLFW.windows.push(win);
        } else {
          GLFW.windows[id - 1] = win;
        }
  
        GLFW.active = win;
        return win.id;
      },destroyWindow:function (winid) {
        var win = GLFW.WindowFromId(winid);
        if (!win) return;
  
        if (win.windowCloseFunc)
          Module['dynCall_vi'](win.windowCloseFunc, win.id);
  
        GLFW.windows[win.id - 1] = null;
        if (GLFW.active.id == win.id)
          GLFW.active = null;
  
        // Destroy context when no alive windows
        for (var i = 0; i < GLFW.windows.length; i++)
          if (GLFW.windows[i] !== null) return;
  
        Module.ctx = Browser.destroyContext(Module['canvas'], true, true);
      },swapBuffers:function (winid) {
      },GLFW2ParamToGLFW3Param:function (param) {
        table = {
          0x00030001:0, // GLFW_MOUSE_CURSOR
          0x00030002:0, // GLFW_STICKY_KEYS
          0x00030003:0, // GLFW_STICKY_MOUSE_BUTTONS
          0x00030004:0, // GLFW_SYSTEM_KEYS
          0x00030005:0, // GLFW_KEY_REPEAT
          0x00030006:0, // GLFW_AUTO_POLL_EVENTS
          0x00020001:0, // GLFW_OPENED
          0x00020002:0, // GLFW_ACTIVE
          0x00020003:0, // GLFW_ICONIFIED
          0x00020004:0, // GLFW_ACCELERATED
          0x00020005:0x00021001, // GLFW_RED_BITS
          0x00020006:0x00021002, // GLFW_GREEN_BITS
          0x00020007:0x00021003, // GLFW_BLUE_BITS
          0x00020008:0x00021004, // GLFW_ALPHA_BITS
          0x00020009:0x00021005, // GLFW_DEPTH_BITS
          0x0002000A:0x00021006, // GLFW_STENCIL_BITS
          0x0002000B:0x0002100F, // GLFW_REFRESH_RATE
          0x0002000C:0x00021007, // GLFW_ACCUM_RED_BITS
          0x0002000D:0x00021008, // GLFW_ACCUM_GREEN_BITS
          0x0002000E:0x00021009, // GLFW_ACCUM_BLUE_BITS
          0x0002000F:0x0002100A, // GLFW_ACCUM_ALPHA_BITS
          0x00020010:0x0002100B, // GLFW_AUX_BUFFERS
          0x00020011:0x0002100C, // GLFW_STEREO
          0x00020012:0, // GLFW_WINDOW_NO_RESIZE
          0x00020013:0x0002100D, // GLFW_FSAA_SAMPLES
          0x00020014:0x00022002, // GLFW_OPENGL_VERSION_MAJOR
          0x00020015:0x00022003, // GLFW_OPENGL_VERSION_MINOR
          0x00020016:0x00022006, // GLFW_OPENGL_FORWARD_COMPAT
          0x00020017:0x00022007, // GLFW_OPENGL_DEBUG_CONTEXT
          0x00020018:0x00022008, // GLFW_OPENGL_PROFILE
        };
        return table[param];
      }};function _glfwGetVideoModes(monitor, count) {
      setValue(count, 0, 'i32');
      return 0;
    }

  function _glLinkProgram(program) {
      GLctx.linkProgram(GL.programs[program]);
      GL.programInfos[program] = null; // uniforms no longer keep the same names after linking
      GL.populateUniformTable(program);
    }

  function _glBindTexture(target, texture) {
      GLctx.bindTexture(target, texture ? GL.textures[texture] : null);
    }

  function _emscripten_glStencilFunc(x0, x1, x2) { GLctx['stencilFunc'](x0, x1, x2) }

  function _glGetString(name_) {
      if (GL.stringCache[name_]) return GL.stringCache[name_];
      var ret; 
      switch(name_) {
        case 0x1F00 /* GL_VENDOR */:
        case 0x1F01 /* GL_RENDERER */:
        case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
        case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
          ret = allocate(intArrayFromString(GLctx.getParameter(name_)), 'i8', ALLOC_NORMAL);
          break;
        case 0x1F02 /* GL_VERSION */:
          var glVersion = GLctx.getParameter(GLctx.VERSION);
          // return GLES version string corresponding to the version of the WebGL context
          {
            glVersion = 'OpenGL ES 2.0 (' + glVersion + ')';
          }
          ret = allocate(intArrayFromString(glVersion), 'i8', ALLOC_NORMAL);
          break;
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = GLctx.getSupportedExtensions();
          var gl_exts = [];
          for (var i = 0; i < exts.length; ++i) {
            gl_exts.push(exts[i]);
            gl_exts.push("GL_" + exts[i]);
          }
          ret = allocate(intArrayFromString(gl_exts.join(' ')), 'i8', ALLOC_NORMAL);
          break;
        case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
          var glslVersion = GLctx.getParameter(GLctx.SHADING_LANGUAGE_VERSION);
          // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
          var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
          var ver_num = glslVersion.match(ver_re);
          if (ver_num !== null) {
            if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
            glslVersion = 'OpenGL ES GLSL ES ' + ver_num[1] + ' (' + glslVersion + ')';
          }
          ret = allocate(intArrayFromString(glslVersion), 'i8', ALLOC_NORMAL);
          break;
        default:
          GL.recordError(0x0500/*GL_INVALID_ENUM*/);
          return 0;
      }
      GL.stringCache[name_] = ret;
      return ret;
    }

  function _emscripten_glUniform3iv(location, count, value) {
  
  
      GLctx.uniform3iv(GL.uniforms[location], HEAP32.subarray((value)>>2,(value+count*12)>>2));
    }

  function _emscripten_glShaderSource(shader, count, string, length) {
      var source = GL.getSource(shader, count, string, length);
  
  
      GLctx.shaderSource(GL.shaders[shader], source);
    }

  function _emscripten_glReleaseShaderCompiler() {
      // NOP (as allowed by GLES 2.0 spec)
    }

  function _glfwSetScrollCallback(winid, cbfun) {
      GLFW.setScrollCallback(winid, cbfun);
    }

  function _emscripten_glTexParameterf(x0, x1, x2) { GLctx['texParameterf'](x0, x1, x2) }

  function _emscripten_glTexParameteri(x0, x1, x2) { GLctx['texParameteri'](x0, x1, x2) }

  function _glCompileShader(shader) {
      GLctx.compileShader(GL.shaders[shader]);
    }

  
  var SYSCALLS={varargs:0,get:function (varargs) {
        SYSCALLS.varargs += 4;
        var ret = HEAP32[(((SYSCALLS.varargs)-(4))>>2)];
        return ret;
      },getStr:function () {
        var ret = Pointer_stringify(SYSCALLS.get());
        return ret;
      },get64:function () {
        var low = SYSCALLS.get(), high = SYSCALLS.get();
        if (low >= 0) assert(high === 0);
        else assert(high === -1);
        return low;
      },getZero:function () {
        assert(SYSCALLS.get() === 0);
      }};function ___syscall54(which, varargs) {SYSCALLS.varargs = varargs;
  try {
   // ioctl
      return 0;
    } catch (e) {
    if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);
    return -e.errno;
  }
  }

  function _emscripten_glSampleCoverage(value, invert) {
      GLctx.sampleCoverage(value, !!invert);
    }

  function _glDeleteTextures(n, textures) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((textures)+(i*4))>>2)];
        var texture = GL.textures[id];
        if (!texture) continue; // GL spec: "glDeleteTextures silently ignores 0s and names that do not correspond to existing textures".
        GLctx.deleteTexture(texture);
        texture.name = 0;
        GL.textures[id] = null;
      }
    }

  function _emscripten_glFrustum() {
  Module['printErr']('missing function: emscripten_glFrustum'); abort(-1);
  }

  function _glfwSetWindowSizeCallback(winid, cbfun) {
      GLFW.setWindowSizeCallback(winid, cbfun);
    }

  function _emscripten_glGetTexParameterfv(target, pname, params) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAPF32[((params)>>2)]=GLctx.getTexParameter(target, pname);
    }

  function _emscripten_glUniform4i(location, v0, v1, v2, v3) {
      GLctx.uniform4i(GL.uniforms[location], v0, v1, v2, v3);
    }

  function _emscripten_glBindRenderbuffer(target, renderbuffer) {
      GLctx.bindRenderbuffer(target, renderbuffer ? GL.renderbuffers[renderbuffer] : null);
    }

  function _emscripten_glViewport(x0, x1, x2, x3) { GLctx['viewport'](x0, x1, x2, x3) }

  
  
  var JSEvents={keyEvent:0,mouseEvent:0,wheelEvent:0,uiEvent:0,focusEvent:0,deviceOrientationEvent:0,deviceMotionEvent:0,fullscreenChangeEvent:0,pointerlockChangeEvent:0,visibilityChangeEvent:0,touchEvent:0,lastGamepadState:null,lastGamepadStateFrame:null,numGamepadsConnected:0,previousFullscreenElement:null,previousScreenX:null,previousScreenY:null,removeEventListenersRegistered:false,staticInit:function () {
        if (typeof window !== 'undefined') {
          window.addEventListener("gamepadconnected", function() { ++JSEvents.numGamepadsConnected; });
          window.addEventListener("gamepaddisconnected", function() { --JSEvents.numGamepadsConnected; });
        }
      },registerRemoveEventListeners:function () {
        if (!JSEvents.removeEventListenersRegistered) {
        __ATEXIT__.push(function() {
            for(var i = JSEvents.eventHandlers.length-1; i >= 0; --i) {
              JSEvents._removeHandler(i);
            }
           });
          JSEvents.removeEventListenersRegistered = true;
        }
      },findEventTarget:function (target) {
        if (target) {
          if (typeof target == "number") {
            target = Pointer_stringify(target);
          }
          if (target == '#window') return window;
          else if (target == '#document') return document;
          else if (target == '#screen') return window.screen;
          else if (target == '#canvas') return Module['canvas'];
  
          if (typeof target == 'string') return document.getElementById(target);
          else return target;
        } else {
          // The sensible target varies between events, but use window as the default
          // since DOM events mostly can default to that. Specific callback registrations
          // override their own defaults.
          return window;
        }
      },deferredCalls:[],deferCall:function (targetFunction, precedence, argsList) {
        function arraysHaveEqualContent(arrA, arrB) {
          if (arrA.length != arrB.length) return false;
  
          for(var i in arrA) {
            if (arrA[i] != arrB[i]) return false;
          }
          return true;
        }
        // Test if the given call was already queued, and if so, don't add it again.
        for(var i in JSEvents.deferredCalls) {
          var call = JSEvents.deferredCalls[i];
          if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
            return;
          }
        }
        JSEvents.deferredCalls.push({
          targetFunction: targetFunction,
          precedence: precedence,
          argsList: argsList
        });
  
        JSEvents.deferredCalls.sort(function(x,y) { return x.precedence < y.precedence; });
      },removeDeferredCalls:function (targetFunction) {
        for(var i = 0; i < JSEvents.deferredCalls.length; ++i) {
          if (JSEvents.deferredCalls[i].targetFunction == targetFunction) {
            JSEvents.deferredCalls.splice(i, 1);
            --i;
          }
        }
      },canPerformEventHandlerRequests:function () {
        return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
      },runDeferredCalls:function () {
        if (!JSEvents.canPerformEventHandlerRequests()) {
          return;
        }
        for(var i = 0; i < JSEvents.deferredCalls.length; ++i) {
          var call = JSEvents.deferredCalls[i];
          JSEvents.deferredCalls.splice(i, 1);
          --i;
          call.targetFunction.apply(this, call.argsList);
        }
      },inEventHandler:0,currentEventHandler:null,eventHandlers:[],isInternetExplorer:function () { return navigator.userAgent.indexOf('MSIE') !== -1 || navigator.appVersion.indexOf('Trident/') > 0; },removeAllHandlersOnTarget:function (target, eventTypeString) {
        for(var i = 0; i < JSEvents.eventHandlers.length; ++i) {
          if (JSEvents.eventHandlers[i].target == target && 
            (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
             JSEvents._removeHandler(i--);
           }
        }
      },_removeHandler:function (i) {
        var h = JSEvents.eventHandlers[i];
        h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
        JSEvents.eventHandlers.splice(i, 1);
      },registerOrRemoveHandler:function (eventHandler) {
        var jsEventHandler = function jsEventHandler(event) {
          // Increment nesting count for the event handler.
          ++JSEvents.inEventHandler;
          JSEvents.currentEventHandler = eventHandler;
          // Process any old deferred calls the user has placed.
          JSEvents.runDeferredCalls();
          // Process the actual event, calls back to user C code handler.
          eventHandler.handlerFunc(event);
          // Process any new deferred calls that were placed right now from this event handler.
          JSEvents.runDeferredCalls();
          // Out of event handler - restore nesting count.
          --JSEvents.inEventHandler;
        }
        
        if (eventHandler.callbackfunc) {
          eventHandler.eventListenerFunc = jsEventHandler;
          eventHandler.target.addEventListener(eventHandler.eventTypeString, jsEventHandler, eventHandler.useCapture);
          JSEvents.eventHandlers.push(eventHandler);
          JSEvents.registerRemoveEventListeners();
        } else {
          for(var i = 0; i < JSEvents.eventHandlers.length; ++i) {
            if (JSEvents.eventHandlers[i].target == eventHandler.target
             && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
               JSEvents._removeHandler(i--);
             }
          }
        }
      },registerKeyEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.keyEvent) {
          JSEvents.keyEvent = _malloc( 164 );
        }
        var handlerFunc = function(event) {
          var e = event || window.event;
          stringToUTF8(e.key ? e.key : "", JSEvents.keyEvent + 0, 32);
          stringToUTF8(e.code ? e.code : "", JSEvents.keyEvent + 32, 32);
          HEAP32[(((JSEvents.keyEvent)+(64))>>2)]=e.location;
          HEAP32[(((JSEvents.keyEvent)+(68))>>2)]=e.ctrlKey;
          HEAP32[(((JSEvents.keyEvent)+(72))>>2)]=e.shiftKey;
          HEAP32[(((JSEvents.keyEvent)+(76))>>2)]=e.altKey;
          HEAP32[(((JSEvents.keyEvent)+(80))>>2)]=e.metaKey;
          HEAP32[(((JSEvents.keyEvent)+(84))>>2)]=e.repeat;
          stringToUTF8(e.locale ? e.locale : "", JSEvents.keyEvent + 88, 32);
          stringToUTF8(e.char ? e.char : "", JSEvents.keyEvent + 120, 32);
          HEAP32[(((JSEvents.keyEvent)+(152))>>2)]=e.charCode;
          HEAP32[(((JSEvents.keyEvent)+(156))>>2)]=e.keyCode;
          HEAP32[(((JSEvents.keyEvent)+(160))>>2)]=e.which;
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.keyEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: JSEvents.isInternetExplorer() ? false : true, // MSIE doesn't allow fullscreen and pointerlock requests from key handlers, others do.
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },getBoundingClientRectOrZeros:function (target) {
        return target.getBoundingClientRect ? target.getBoundingClientRect() : { left: 0, top: 0 };
      },fillMouseEventData:function (eventStruct, e, target) {
        HEAPF64[((eventStruct)>>3)]=JSEvents.tick();
        HEAP32[(((eventStruct)+(8))>>2)]=e.screenX;
        HEAP32[(((eventStruct)+(12))>>2)]=e.screenY;
        HEAP32[(((eventStruct)+(16))>>2)]=e.clientX;
        HEAP32[(((eventStruct)+(20))>>2)]=e.clientY;
        HEAP32[(((eventStruct)+(24))>>2)]=e.ctrlKey;
        HEAP32[(((eventStruct)+(28))>>2)]=e.shiftKey;
        HEAP32[(((eventStruct)+(32))>>2)]=e.altKey;
        HEAP32[(((eventStruct)+(36))>>2)]=e.metaKey;
        HEAP16[(((eventStruct)+(40))>>1)]=e.button;
        HEAP16[(((eventStruct)+(42))>>1)]=e.buttons;
        HEAP32[(((eventStruct)+(44))>>2)]=e["movementX"] || e["mozMovementX"] || e["webkitMovementX"] || (e.screenX-JSEvents.previousScreenX);
        HEAP32[(((eventStruct)+(48))>>2)]=e["movementY"] || e["mozMovementY"] || e["webkitMovementY"] || (e.screenY-JSEvents.previousScreenY);
  
        if (Module['canvas']) {
          var rect = Module['canvas'].getBoundingClientRect();
          HEAP32[(((eventStruct)+(60))>>2)]=e.clientX - rect.left;
          HEAP32[(((eventStruct)+(64))>>2)]=e.clientY - rect.top;
        } else { // Canvas is not initialized, return 0.
          HEAP32[(((eventStruct)+(60))>>2)]=0;
          HEAP32[(((eventStruct)+(64))>>2)]=0;
        }
        if (target) {
          var rect = JSEvents.getBoundingClientRectOrZeros(target);
          HEAP32[(((eventStruct)+(52))>>2)]=e.clientX - rect.left;
          HEAP32[(((eventStruct)+(56))>>2)]=e.clientY - rect.top;        
        } else { // No specific target passed, return 0.
          HEAP32[(((eventStruct)+(52))>>2)]=0;
          HEAP32[(((eventStruct)+(56))>>2)]=0;
        }
        // wheel and mousewheel events contain wrong screenX/screenY on chrome/opera
        // https://github.com/kripken/emscripten/pull/4997
        // https://bugs.chromium.org/p/chromium/issues/detail?id=699956
        if (e.type !== 'wheel' && e.type !== 'mousewheel') {
          JSEvents.previousScreenX = e.screenX;
          JSEvents.previousScreenY = e.screenY;
        }
      },registerMouseEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.mouseEvent) {
          JSEvents.mouseEvent = _malloc( 72 );
        }
        target = JSEvents.findEventTarget(target);
        var handlerFunc = function(event) {
          var e = event || window.event;
          JSEvents.fillMouseEventData(JSEvents.mouseEvent, e, target);
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.mouseEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        // In IE, mousedown events don't either allow deferred calls to be run!
        if (JSEvents.isInternetExplorer() && eventTypeString == 'mousedown') eventHandler.allowsDeferredCalls = false;
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerWheelEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.wheelEvent) {
          JSEvents.wheelEvent = _malloc( 104 );
        }
        target = JSEvents.findEventTarget(target);
        // The DOM Level 3 events spec event 'wheel'
        var wheelHandlerFunc = function(event) {
          var e = event || window.event;
          JSEvents.fillMouseEventData(JSEvents.wheelEvent, e, target);
          HEAPF64[(((JSEvents.wheelEvent)+(72))>>3)]=e["deltaX"];
          HEAPF64[(((JSEvents.wheelEvent)+(80))>>3)]=e["deltaY"];
          HEAPF64[(((JSEvents.wheelEvent)+(88))>>3)]=e["deltaZ"];
          HEAP32[(((JSEvents.wheelEvent)+(96))>>2)]=e["deltaMode"];
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.wheelEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
        // The 'mousewheel' event as implemented in Safari 6.0.5
        var mouseWheelHandlerFunc = function(event) {
          var e = event || window.event;
          JSEvents.fillMouseEventData(JSEvents.wheelEvent, e, target);
          HEAPF64[(((JSEvents.wheelEvent)+(72))>>3)]=e["wheelDeltaX"] || 0;
          HEAPF64[(((JSEvents.wheelEvent)+(80))>>3)]=-(e["wheelDeltaY"] ? e["wheelDeltaY"] : e["wheelDelta"]) /* 1. Invert to unify direction with the DOM Level 3 wheel event. 2. MSIE does not provide wheelDeltaY, so wheelDelta is used as a fallback. */;
          HEAPF64[(((JSEvents.wheelEvent)+(88))>>3)]=0 /* Not available */;
          HEAP32[(((JSEvents.wheelEvent)+(96))>>2)]=0 /* DOM_DELTA_PIXEL */;
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.wheelEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: true,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: (eventTypeString == 'wheel') ? wheelHandlerFunc : mouseWheelHandlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },pageScrollPos:function () {
        if (window.pageXOffset > 0 || window.pageYOffset > 0) {
          return [window.pageXOffset, window.pageYOffset];
        }
        if (typeof document.documentElement.scrollLeft !== 'undefined' || typeof document.documentElement.scrollTop !== 'undefined') {
          return [document.documentElement.scrollLeft, document.documentElement.scrollTop];
        }
        return [document.body.scrollLeft|0, document.body.scrollTop|0];
      },registerUiEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.uiEvent) {
          JSEvents.uiEvent = _malloc( 36 );
        }
  
        if (eventTypeString == "scroll" && !target) {
          target = document; // By default read scroll events on document rather than window.
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
          if (e.target != target) {
            // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
            // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
            // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
            // causing a new scroll, etc..
            return;
          }
          var scrollPos = JSEvents.pageScrollPos();
          HEAP32[((JSEvents.uiEvent)>>2)]=e.detail;
          HEAP32[(((JSEvents.uiEvent)+(4))>>2)]=document.body.clientWidth;
          HEAP32[(((JSEvents.uiEvent)+(8))>>2)]=document.body.clientHeight;
          HEAP32[(((JSEvents.uiEvent)+(12))>>2)]=window.innerWidth;
          HEAP32[(((JSEvents.uiEvent)+(16))>>2)]=window.innerHeight;
          HEAP32[(((JSEvents.uiEvent)+(20))>>2)]=window.outerWidth;
          HEAP32[(((JSEvents.uiEvent)+(24))>>2)]=window.outerHeight;
          HEAP32[(((JSEvents.uiEvent)+(28))>>2)]=scrollPos[0];
          HEAP32[(((JSEvents.uiEvent)+(32))>>2)]=scrollPos[1];
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.uiEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false, // Neither scroll or resize events allow running requests inside them.
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },getNodeNameForTarget:function (target) {
        if (!target) return '';
        if (target == window) return '#window';
        if (target == window.screen) return '#screen';
        return (target && target.nodeName) ? target.nodeName : '';
      },registerFocusEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.focusEvent) {
          JSEvents.focusEvent = _malloc( 256 );
        }
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          var nodeName = JSEvents.getNodeNameForTarget(e.target);
          var id = e.target.id ? e.target.id : '';
          stringToUTF8(nodeName, JSEvents.focusEvent + 0, 128);
          stringToUTF8(id, JSEvents.focusEvent + 128, 128);
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.focusEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },tick:function () {
        if (window['performance'] && window['performance']['now']) return window['performance']['now']();
        else return Date.now();
      },registerDeviceOrientationEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.deviceOrientationEvent) {
          JSEvents.deviceOrientationEvent = _malloc( 40 );
        }
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          HEAPF64[((JSEvents.deviceOrientationEvent)>>3)]=JSEvents.tick();
          HEAPF64[(((JSEvents.deviceOrientationEvent)+(8))>>3)]=e.alpha;
          HEAPF64[(((JSEvents.deviceOrientationEvent)+(16))>>3)]=e.beta;
          HEAPF64[(((JSEvents.deviceOrientationEvent)+(24))>>3)]=e.gamma;
          HEAP32[(((JSEvents.deviceOrientationEvent)+(32))>>2)]=e.absolute;
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.deviceOrientationEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerDeviceMotionEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.deviceMotionEvent) {
          JSEvents.deviceMotionEvent = _malloc( 80 );
        }
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          HEAPF64[((JSEvents.deviceOrientationEvent)>>3)]=JSEvents.tick();
          HEAPF64[(((JSEvents.deviceMotionEvent)+(8))>>3)]=e.acceleration.x;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(16))>>3)]=e.acceleration.y;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(24))>>3)]=e.acceleration.z;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(32))>>3)]=e.accelerationIncludingGravity.x;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(40))>>3)]=e.accelerationIncludingGravity.y;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(48))>>3)]=e.accelerationIncludingGravity.z;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(56))>>3)]=e.rotationRate.alpha;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(64))>>3)]=e.rotationRate.beta;
          HEAPF64[(((JSEvents.deviceMotionEvent)+(72))>>3)]=e.rotationRate.gamma;
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.deviceMotionEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },screenOrientation:function () {
        if (!window.screen) return undefined;
        return window.screen.orientation || window.screen.mozOrientation || window.screen.webkitOrientation || window.screen.msOrientation;
      },fillOrientationChangeEventData:function (eventStruct, e) {
        var orientations  = ["portrait-primary", "portrait-secondary", "landscape-primary", "landscape-secondary"];
        var orientations2 = ["portrait",         "portrait",           "landscape",         "landscape"];
  
        var orientationString = JSEvents.screenOrientation();
        var orientation = orientations.indexOf(orientationString);
        if (orientation == -1) {
          orientation = orientations2.indexOf(orientationString);
        }
  
        HEAP32[((eventStruct)>>2)]=1 << orientation;
        HEAP32[(((eventStruct)+(4))>>2)]=window.orientation;
      },registerOrientationChangeEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.orientationChangeEvent) {
          JSEvents.orientationChangeEvent = _malloc( 8 );
        }
  
        if (!target) {
          target = window.screen; // Orientation events need to be captured from 'window.screen' instead of 'window'
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillOrientationChangeEventData(JSEvents.orientationChangeEvent, e);
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.orientationChangeEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        if (eventTypeString == "orientationchange" && window.screen.mozOrientation !== undefined) {
          eventTypeString = "mozorientationchange";
        }
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },fullscreenEnabled:function () {
        return document.fullscreenEnabled || document.mozFullScreenEnabled || document.webkitFullscreenEnabled || document.msFullscreenEnabled;
      },fillFullscreenChangeEventData:function (eventStruct, e) {
        var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
        var isFullscreen = !!fullscreenElement;
        HEAP32[((eventStruct)>>2)]=isFullscreen;
        HEAP32[(((eventStruct)+(4))>>2)]=JSEvents.fullscreenEnabled();
        // If transitioning to fullscreen, report info about the element that is now fullscreen.
        // If transitioning to windowed mode, report info about the element that just was fullscreen.
        var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
        var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
        var id = (reportedElement && reportedElement.id) ? reportedElement.id : '';
        stringToUTF8(nodeName, eventStruct + 8, 128);
        stringToUTF8(id, eventStruct + 136, 128);
        HEAP32[(((eventStruct)+(264))>>2)]=reportedElement ? reportedElement.clientWidth : 0;
        HEAP32[(((eventStruct)+(268))>>2)]=reportedElement ? reportedElement.clientHeight : 0;
        HEAP32[(((eventStruct)+(272))>>2)]=screen.width;
        HEAP32[(((eventStruct)+(276))>>2)]=screen.height;
        if (isFullscreen) {
          JSEvents.previousFullscreenElement = fullscreenElement;
        }
      },registerFullscreenChangeEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.fullscreenChangeEvent) {
          JSEvents.fullscreenChangeEvent = _malloc( 280 );
        }
  
        if (!target) {
          target = document; // Fullscreen change events need to be captured from 'document' by default instead of 'window'
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillFullscreenChangeEventData(JSEvents.fullscreenChangeEvent, e);
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.fullscreenChangeEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },resizeCanvasForFullscreen:function (target, strategy) {
        var restoreOldStyle = __registerRestoreOldStyle(target);
        var cssWidth = strategy.softFullscreen ? window.innerWidth : screen.width;
        var cssHeight = strategy.softFullscreen ? window.innerHeight : screen.height;
        var rect = target.getBoundingClientRect();
        var windowedCssWidth = rect.right - rect.left;
        var windowedCssHeight = rect.bottom - rect.top;
        var windowedRttWidth = target.width;
        var windowedRttHeight = target.height;
  
        if (strategy.scaleMode == 3) {
          __setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
          cssWidth = windowedCssWidth;
          cssHeight = windowedCssHeight;
        } else if (strategy.scaleMode == 2) {
          if (cssWidth*windowedRttHeight < windowedRttWidth*cssHeight) {
            var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
            __setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
            cssHeight = desiredCssHeight;
          } else {
            var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
            __setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
            cssWidth = desiredCssWidth;
          }
        }
  
        // If we are adding padding, must choose a background color or otherwise Chrome will give the
        // padding a default white color. Do it only if user has not customized their own background color.
        if (!target.style.backgroundColor) target.style.backgroundColor = 'black';
        // IE11 does the same, but requires the color to be set in the document body.
        if (!document.body.style.backgroundColor) document.body.style.backgroundColor = 'black'; // IE11
        // Firefox always shows black letterboxes independent of style color.
  
        target.style.width = cssWidth + 'px';
        target.style.height = cssHeight + 'px';
  
        if (strategy.filteringMode == 1) {
          target.style.imageRendering = 'optimizeSpeed';
          target.style.imageRendering = '-moz-crisp-edges';
          target.style.imageRendering = '-o-crisp-edges';
          target.style.imageRendering = '-webkit-optimize-contrast';
          target.style.imageRendering = 'optimize-contrast';
          target.style.imageRendering = 'crisp-edges';
          target.style.imageRendering = 'pixelated';
        }
  
        var dpiScale = (strategy.canvasResolutionScaleMode == 2) ? window.devicePixelRatio : 1;
        if (strategy.canvasResolutionScaleMode != 0) {
          target.width = cssWidth * dpiScale;
          target.height = cssHeight * dpiScale;
          if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, target.width, target.height);
        }
        return restoreOldStyle;
      },requestFullscreen:function (target, strategy) {
        // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
        if (strategy.scaleMode != 0 || strategy.canvasResolutionScaleMode != 0) {
          JSEvents.resizeCanvasForFullscreen(target, strategy);
        }
  
        if (target.requestFullscreen) {
          target.requestFullscreen();
        } else if (target.msRequestFullscreen) {
          target.msRequestFullscreen();
        } else if (target.mozRequestFullScreen) {
          target.mozRequestFullScreen();
        } else if (target.mozRequestFullscreen) {
          target.mozRequestFullscreen();
        } else if (target.webkitRequestFullscreen) {
          target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
        } else {
          if (typeof JSEvents.fullscreenEnabled() === 'undefined') {
            return -1;
          } else {
            return -3;
          }
        }
  
        if (strategy.canvasResizedCallback) {
          Module['dynCall_iiii'](strategy.canvasResizedCallback, 37, 0, strategy.canvasResizedCallbackUserData);
        }
  
        return 0;
      },fillPointerlockChangeEventData:function (eventStruct, e) {
        var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
        var isPointerlocked = !!pointerLockElement;
        HEAP32[((eventStruct)>>2)]=isPointerlocked;
        var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
        var id = (pointerLockElement && pointerLockElement.id) ? pointerLockElement.id : '';
        stringToUTF8(nodeName, eventStruct + 4, 128);
        stringToUTF8(id, eventStruct + 132, 128);
      },registerPointerlockChangeEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.pointerlockChangeEvent) {
          JSEvents.pointerlockChangeEvent = _malloc( 260 );
        }
  
        if (!target) {
          target = document; // Pointer lock change events need to be captured from 'document' by default instead of 'window'
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillPointerlockChangeEventData(JSEvents.pointerlockChangeEvent, e);
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.pointerlockChangeEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerPointerlockErrorEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!target) {
          target = document; // Pointer lock events need to be captured from 'document' by default instead of 'window'
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },requestPointerLock:function (target) {
        if (target.requestPointerLock) {
          target.requestPointerLock();
        } else if (target.mozRequestPointerLock) {
          target.mozRequestPointerLock();
        } else if (target.webkitRequestPointerLock) {
          target.webkitRequestPointerLock();
        } else if (target.msRequestPointerLock) {
          target.msRequestPointerLock();
        } else {
          // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
          // or if the whole browser just doesn't support the feature.
          if (document.body.requestPointerLock || document.body.mozRequestPointerLock || document.body.webkitRequestPointerLock || document.body.msRequestPointerLock) {
            return -3;
          } else {
            return -1;
          }
        }
        return 0;
      },fillVisibilityChangeEventData:function (eventStruct, e) {
        var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
        var visibilityState = visibilityStates.indexOf(document.visibilityState);
  
        HEAP32[((eventStruct)>>2)]=document.hidden;
        HEAP32[(((eventStruct)+(4))>>2)]=visibilityState;
      },registerVisibilityChangeEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.visibilityChangeEvent) {
          JSEvents.visibilityChangeEvent = _malloc( 8 );
        }
  
        if (!target) {
          target = document; // Visibility change events need to be captured from 'document' by default instead of 'window'
        } else {
          target = JSEvents.findEventTarget(target);
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillVisibilityChangeEventData(JSEvents.visibilityChangeEvent, e);
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.visibilityChangeEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerTouchEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.touchEvent) {
          JSEvents.touchEvent = _malloc( 1684 );
        }
  
        target = JSEvents.findEventTarget(target);
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          var touches = {};
          for(var i = 0; i < e.touches.length; ++i) {
            var touch = e.touches[i];
            touches[touch.identifier] = touch;
          }
          for(var i = 0; i < e.changedTouches.length; ++i) {
            var touch = e.changedTouches[i];
            touches[touch.identifier] = touch;
            touch.changed = true;
          }
          for(var i = 0; i < e.targetTouches.length; ++i) {
            var touch = e.targetTouches[i];
            touches[touch.identifier].onTarget = true;
          }
          
          var ptr = JSEvents.touchEvent;
          HEAP32[(((ptr)+(4))>>2)]=e.ctrlKey;
          HEAP32[(((ptr)+(8))>>2)]=e.shiftKey;
          HEAP32[(((ptr)+(12))>>2)]=e.altKey;
          HEAP32[(((ptr)+(16))>>2)]=e.metaKey;
          ptr += 20; // Advance to the start of the touch array.
          var canvasRect = Module['canvas'] ? Module['canvas'].getBoundingClientRect() : undefined;
          var targetRect = JSEvents.getBoundingClientRectOrZeros(target);
          var numTouches = 0;
          for(var i in touches) {
            var t = touches[i];
            HEAP32[((ptr)>>2)]=t.identifier;
            HEAP32[(((ptr)+(4))>>2)]=t.screenX;
            HEAP32[(((ptr)+(8))>>2)]=t.screenY;
            HEAP32[(((ptr)+(12))>>2)]=t.clientX;
            HEAP32[(((ptr)+(16))>>2)]=t.clientY;
            HEAP32[(((ptr)+(20))>>2)]=t.pageX;
            HEAP32[(((ptr)+(24))>>2)]=t.pageY;
            HEAP32[(((ptr)+(28))>>2)]=t.changed;
            HEAP32[(((ptr)+(32))>>2)]=t.onTarget;
            if (canvasRect) {
              HEAP32[(((ptr)+(44))>>2)]=t.clientX - canvasRect.left;
              HEAP32[(((ptr)+(48))>>2)]=t.clientY - canvasRect.top;
            } else {
              HEAP32[(((ptr)+(44))>>2)]=0;
              HEAP32[(((ptr)+(48))>>2)]=0;            
            }
            HEAP32[(((ptr)+(36))>>2)]=t.clientX - targetRect.left;
            HEAP32[(((ptr)+(40))>>2)]=t.clientY - targetRect.top;
            
            ptr += 52;
  
            if (++numTouches >= 32) {
              break;
            }
          }
          HEAP32[((JSEvents.touchEvent)>>2)]=numTouches;
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.touchEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: target,
          allowsDeferredCalls: false, // XXX Currently disabled, see bug https://bugzilla.mozilla.org/show_bug.cgi?id=966493
          // Once the above bug is resolved, enable the following condition if possible:
          // allowsDeferredCalls: eventTypeString == 'touchstart',
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },fillGamepadEventData:function (eventStruct, e) {
        HEAPF64[((eventStruct)>>3)]=e.timestamp;
        for(var i = 0; i < e.axes.length; ++i) {
          HEAPF64[(((eventStruct+i*8)+(16))>>3)]=e.axes[i];
        }
        for(var i = 0; i < e.buttons.length; ++i) {
          if (typeof(e.buttons[i]) === 'object') {
            HEAPF64[(((eventStruct+i*8)+(528))>>3)]=e.buttons[i].value;
          } else {
            HEAPF64[(((eventStruct+i*8)+(528))>>3)]=e.buttons[i];
          }
        }
        for(var i = 0; i < e.buttons.length; ++i) {
          if (typeof(e.buttons[i]) === 'object') {
            HEAP32[(((eventStruct+i*4)+(1040))>>2)]=e.buttons[i].pressed;
          } else {
            HEAP32[(((eventStruct+i*4)+(1040))>>2)]=e.buttons[i] == 1.0;
          }
        }
        HEAP32[(((eventStruct)+(1296))>>2)]=e.connected;
        HEAP32[(((eventStruct)+(1300))>>2)]=e.index;
        HEAP32[(((eventStruct)+(8))>>2)]=e.axes.length;
        HEAP32[(((eventStruct)+(12))>>2)]=e.buttons.length;
        stringToUTF8(e.id, eventStruct + 1304, 64);
        stringToUTF8(e.mapping, eventStruct + 1368, 64);
      },registerGamepadEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.gamepadEvent) {
          JSEvents.gamepadEvent = _malloc( 1432 );
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillGamepadEventData(JSEvents.gamepadEvent, e.gamepad);
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.gamepadEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: true,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerBeforeUnloadEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          var confirmationMessage = Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData);
          
          if (confirmationMessage) {
            confirmationMessage = Pointer_stringify(confirmationMessage);
          }
          if (confirmationMessage) {
            e.preventDefault();
            e.returnValue = confirmationMessage;
            return confirmationMessage;
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },battery:function () { return navigator.battery || navigator.mozBattery || navigator.webkitBattery; },fillBatteryEventData:function (eventStruct, e) {
        HEAPF64[((eventStruct)>>3)]=e.chargingTime;
        HEAPF64[(((eventStruct)+(8))>>3)]=e.dischargingTime;
        HEAPF64[(((eventStruct)+(16))>>3)]=e.level;
        HEAP32[(((eventStruct)+(24))>>2)]=e.charging;
      },registerBatteryEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!JSEvents.batteryEvent) {
          JSEvents.batteryEvent = _malloc( 32 );
        }
  
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          JSEvents.fillBatteryEventData(JSEvents.batteryEvent, JSEvents.battery());
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, JSEvents.batteryEvent, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      },registerWebGlEventCallback:function (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) {
        if (!target) {
          target = Module['canvas'];
        }
        var handlerFunc = function(event) {
          var e = event || window.event;
  
          var shouldCancel = Module['dynCall_iiii'](callbackfunc, eventTypeId, 0, userData);
          if (shouldCancel) {
            e.preventDefault();
          }
        };
  
        var eventHandler = {
          target: JSEvents.findEventTarget(target),
          allowsDeferredCalls: false,
          eventTypeString: eventTypeString,
          callbackfunc: callbackfunc,
          handlerFunc: handlerFunc,
          useCapture: useCapture
        };
        JSEvents.registerOrRemoveHandler(eventHandler);
      }};function __emscripten_sample_gamepad_data() {
      // Polling gamepads generates garbage, so don't do it when we know there are no gamepads connected.
      if (!JSEvents.numGamepadsConnected) return;
  
      // Produce a new Gamepad API sample if we are ticking a new game frame, or if not using emscripten_set_main_loop() at all to drive animation.
      if (Browser.mainLoop.currentFrameNumber !== JSEvents.lastGamepadStateFrame || !Browser.mainLoop.currentFrameNumber) {
        JSEvents.lastGamepadState = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : null);
        JSEvents.lastGamepadStateFrame = Browser.mainLoop.currentFrameNumber;
      }
    }function _emscripten_get_gamepad_status(index, gamepadState) {
      __emscripten_sample_gamepad_data();
      if (!JSEvents.lastGamepadState) return -1;
  
      // INVALID_PARAM is returned on a Gamepad index that never was there.
      if (index < 0 || index >= JSEvents.lastGamepadState.length) return -5;
  
      // NO_DATA is returned on a Gamepad index that was removed.
      // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
      // This is because gamepads must keep their original position in the array.
      // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
      if (!JSEvents.lastGamepadState[index]) return -7;
  
      JSEvents.fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
      return 0;
    }

  function _emscripten_glCopyTexImage2D(x0, x1, x2, x3, x4, x5, x6, x7) { GLctx['copyTexImage2D'](x0, x1, x2, x3, x4, x5, x6, x7) }

  function _emscripten_glTexParameterfv(target, pname, params) {
      var param = HEAPF32[((params)>>2)];
      GLctx.texParameterf(target, pname, param);
    }

  function _emscripten_glLinkProgram(program) {
      GLctx.linkProgram(GL.programs[program]);
      GL.programInfos[program] = null; // uniforms no longer keep the same names after linking
      GL.populateUniformTable(program);
    }

  function _emscripten_glUniform3f(location, v0, v1, v2) {
      GLctx.uniform3f(GL.uniforms[location], v0, v1, v2);
    }

  function _emscripten_glGetObjectParameterivARB() {
  Module['printErr']('missing function: emscripten_glGetObjectParameterivARB'); abort(-1);
  }

  function _emscripten_glBlendFunc(x0, x1) { GLctx['blendFunc'](x0, x1) }

  function _emscripten_glUniform3i(location, v0, v1, v2) {
      GLctx.uniform3i(GL.uniforms[location], v0, v1, v2);
    }

  function _emscripten_glStencilOp(x0, x1, x2) { GLctx['stencilOp'](x0, x1, x2) }

  function _glCreateShader(shaderType) {
      var id = GL.getNewId(GL.shaders);
      GL.shaders[id] = GLctx.createShader(shaderType);
      return id;
    }

  function _glUniform1i(location, v0) {
      GLctx.uniform1i(GL.uniforms[location], v0);
    }

  function _emscripten_glBindAttribLocation(program, index, name) {
      name = Pointer_stringify(name);
      GLctx.bindAttribLocation(GL.programs[program], index, name);
    }

  function _glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data) {
      GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data ? HEAPU8.subarray((data),(data+imageSize)) : null);
    }

  function _glDisable(x0) { GLctx['disable'](x0) }

  function _emscripten_glEnableVertexAttribArray(index) {
      GLctx.enableVertexAttribArray(index);
    }

   
  Module["_memset"] = _memset;

  function _glfwMakeContextCurrent(winid) {}

  function _emscripten_set_touchcancel_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, 25, "touchcancel");
      return 0;
    }

  function ___lock() {}

  function _emscripten_glBlendFuncSeparate(x0, x1, x2, x3) { GLctx['blendFuncSeparate'](x0, x1, x2, x3) }

  function _glCullFace(x0) { GLctx['cullFace'](x0) }

  function _emscripten_glGetVertexAttribPointerv(index, pname, pointer) {
      if (!pointer) {
        // GLES2 specification does not specify how to behave if pointer is a null pointer. Since calling this function does not make sense
        // if pointer == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((pointer)>>2)]=GLctx.getVertexAttribOffset(index, pname);
    }

  function _emscripten_glVertexAttrib3f(x0, x1, x2, x3) { GLctx['vertexAttrib3f'](x0, x1, x2, x3) }

  function _emscripten_glEnable(x0) { GLctx['enable'](x0) }

  function _emscripten_glNormalPointer() {
  Module['printErr']('missing function: emscripten_glNormalPointer'); abort(-1);
  }

  
  var _emscripten_GetProcAddress=undefined;
  Module["_emscripten_GetProcAddress"] = _emscripten_GetProcAddress;
  
  var EGL={errorCode:12288,defaultDisplayInitialized:false,currentContext:0,currentReadSurface:0,currentDrawSurface:0,stringCache:{},setErrorCode:function (code) {
        EGL.errorCode = code;
      },chooseConfig:function (display, attribList, config, config_size, numConfigs) { 
        if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
          EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
          return 0;
        }
        // TODO: read attribList.
        if ((!config || !config_size) && !numConfigs) {
          EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
          return 0;
        }
        if (numConfigs) {
          HEAP32[((numConfigs)>>2)]=1; // Total number of supported configs: 1.
        }
        if (config && config_size > 0) {
          HEAP32[((config)>>2)]=62002; 
        }
        
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1;
      }};function _eglGetProcAddress(name_) {
      return _emscripten_GetProcAddress(name_);
    }

  function _glDeleteProgram(id) {
      if (!id) return;
      var program = GL.programs[id];
      if (!program) { // glDeleteProgram actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteProgram(program);
      program.name = 0;
      GL.programs[id] = null;
      GL.programInfos[id] = null;
    }

  function _emscripten_get_pointerlock_status(pointerlockStatus) {
      if (pointerlockStatus) JSEvents.fillPointerlockChangeEventData(pointerlockStatus);
      if (!document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
        return -1;
      }
      return 0;
    }

  function _glAttachShader(program, shader) {
      GLctx.attachShader(GL.programs[program],
                              GL.shaders[shader]);
    }

  function _glfwGetPrimaryMonitor() {
      return 1;
    }

  
  function emscriptenWebGLGetVertexAttrib(index, pname, params, type) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      var data = GLctx.getVertexAttrib(index, pname);
      if (pname == 0x889F/*VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/) {
        HEAP32[((params)>>2)]=data["name"];
      } else if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 'Integer': HEAP32[((params)>>2)]=data; break;
          case 'Float': HEAPF32[((params)>>2)]=data; break;
          case 'FloatToInteger': HEAP32[((params)>>2)]=Math.fround(data); break;
          default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 'Integer': HEAP32[(((params)+(i))>>2)]=data[i]; break;
            case 'Float': HEAPF32[(((params)+(i))>>2)]=data[i]; break;
            case 'FloatToInteger': HEAP32[(((params)+(i))>>2)]=Math.fround(data[i]); break;
            default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
          }
        }
      }
    }function _emscripten_glGetVertexAttribfv(index, pname, params) {
      // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttrib*f(),
      // otherwise the results are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 'Float');
    }

  function _emscripten_set_touchstart_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, 22, "touchstart");
      return 0;
    }

  function _emscripten_glDeleteShader(id) {
      if (!id) return;
      var shader = GL.shaders[id];
      if (!shader) { // glDeleteShader actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteShader(shader);
      GL.shaders[id] = null;
    }

  function _emscripten_glVertexPointer(){ throw 'Legacy GL function (glVertexPointer) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; }

  function _emscripten_glDeleteBuffers(n, buffers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((buffers)+(i*4))>>2)];
        var buffer = GL.buffers[id];
  
        // From spec: "glDeleteBuffers silently ignores 0's and names that do not
        // correspond to existing buffer objects."
        if (!buffer) continue;
  
        GLctx.deleteBuffer(buffer);
        buffer.name = 0;
        GL.buffers[id] = null;
  
        if (id == GL.currArrayBuffer) GL.currArrayBuffer = 0;
        if (id == GL.currElementArrayBuffer) GL.currElementArrayBuffer = 0;
      }
    }

  function _emscripten_glTexParameteriv(target, pname, params) {
      var param = HEAP32[((params)>>2)];
      GLctx.texParameteri(target, pname, param);
    }

  function _glDrawElements(mode, count, type, indices) {
  
      GLctx.drawElements(mode, count, type, indices);
  
    }

  function _glfwTerminate() {
      window.removeEventListener("keydown", GLFW.onKeydown, true);
      window.removeEventListener("keypress", GLFW.onKeyPress, true);
      window.removeEventListener("keyup", GLFW.onKeyup, true);
      Module["canvas"].removeEventListener("mousemove", GLFW.onMousemove, true);
      Module["canvas"].removeEventListener("mousedown", GLFW.onMouseButtonDown, true);
      Module["canvas"].removeEventListener("mouseup", GLFW.onMouseButtonUp, true);
      Module["canvas"].removeEventListener('wheel', GLFW.onMouseWheel, true);
      Module["canvas"].removeEventListener('mousewheel', GLFW.onMouseWheel, true);
      Module["canvas"].removeEventListener('mouseenter', GLFW.onMouseenter, true);
      Module["canvas"].removeEventListener('mouseleave', GLFW.onMouseleave, true);
      Module["canvas"].width = Module["canvas"].height = 1;
      GLFW.windows = null;
      GLFW.active = null;
    }

  function _emscripten_glUniformMatrix2fv(location, count, transpose, value) {
  
  
      var view;
      if (4*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[4*count-1];
        for (var i = 0; i < 4*count; i += 4) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*16)>>2);
      }
      GLctx.uniformMatrix2fv(GL.uniforms[location], !!transpose, view);
    }

  function ___syscall6(which, varargs) {SYSCALLS.varargs = varargs;
  try {
   // close
      var stream = SYSCALLS.getStreamFromFD();
      FS.close(stream);
      return 0;
    } catch (e) {
    if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);
    return -e.errno;
  }
  }

  function _llvm_stacksave() {
      var self = _llvm_stacksave;
      if (!self.LLVM_SAVEDSTACKS) {
        self.LLVM_SAVEDSTACKS = [];
      }
      self.LLVM_SAVEDSTACKS.push(Runtime.stackSave());
      return self.LLVM_SAVEDSTACKS.length-1;
    }

  function _emscripten_glGetVertexAttribiv(index, pname, params) {
      // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttrib*f(),
      // otherwise the results are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 'FloatToInteger');
    }

  function _emscripten_glUniformMatrix4fv(location, count, transpose, value) {
  
  
      var view;
      if (16*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[16*count-1];
        for (var i = 0; i < 16*count; i += 16) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
          view[i+4] = HEAPF32[(((value)+(4*i+16))>>2)];
          view[i+5] = HEAPF32[(((value)+(4*i+20))>>2)];
          view[i+6] = HEAPF32[(((value)+(4*i+24))>>2)];
          view[i+7] = HEAPF32[(((value)+(4*i+28))>>2)];
          view[i+8] = HEAPF32[(((value)+(4*i+32))>>2)];
          view[i+9] = HEAPF32[(((value)+(4*i+36))>>2)];
          view[i+10] = HEAPF32[(((value)+(4*i+40))>>2)];
          view[i+11] = HEAPF32[(((value)+(4*i+44))>>2)];
          view[i+12] = HEAPF32[(((value)+(4*i+48))>>2)];
          view[i+13] = HEAPF32[(((value)+(4*i+52))>>2)];
          view[i+14] = HEAPF32[(((value)+(4*i+56))>>2)];
          view[i+15] = HEAPF32[(((value)+(4*i+60))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*64)>>2);
      }
      GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, view);
    }

  function _emscripten_glDrawArraysInstanced(mode, first, count, primcount) {
      GLctx['drawArraysInstanced'](mode, first, count, primcount);
    }

  function _emscripten_glEnableClientState() {
  Module['printErr']('missing function: emscripten_glEnableClientState'); abort(-1);
  }

  function _emscripten_glGetPointerv() {
  Module['printErr']('missing function: emscripten_glGetPointerv'); abort(-1);
  }

  function ___syscall140(which, varargs) {SYSCALLS.varargs = varargs;
  try {
   // llseek
      var stream = SYSCALLS.getStreamFromFD(), offset_high = SYSCALLS.get(), offset_low = SYSCALLS.get(), result = SYSCALLS.get(), whence = SYSCALLS.get();
      var offset = offset_low;
      assert(offset_high === 0);
      FS.llseek(stream, offset, whence);
      HEAP32[((result)>>2)]=stream.position;
      if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null; // reset readdir state
      return 0;
    } catch (e) {
    if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);
    return -e.errno;
  }
  }

  function ___syscall146(which, varargs) {SYSCALLS.varargs = varargs;
  try {
   // writev
      // hack to support printf in NO_FILESYSTEM
      var stream = SYSCALLS.get(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
      var ret = 0;
      if (!___syscall146.buffer) {
        ___syscall146.buffers = [null, [], []]; // 1 => stdout, 2 => stderr
        ___syscall146.printChar = function(stream, curr) {
          var buffer = ___syscall146.buffers[stream];
          assert(buffer);
          if (curr === 0 || curr === 10) {
            (stream === 1 ? Module['print'] : Module['printErr'])(UTF8ArrayToString(buffer, 0));
            buffer.length = 0;
          } else {
            buffer.push(curr);
          }
        };
      }
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAP32[(((iov)+(i*8))>>2)];
        var len = HEAP32[(((iov)+(i*8 + 4))>>2)];
        for (var j = 0; j < len; j++) {
          ___syscall146.printChar(stream, HEAPU8[ptr+j]);
        }
        ret += len;
      }
      return ret;
    } catch (e) {
    if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);
    return -e.errno;
  }
  }

  function _emscripten_glUniform1i(location, v0) {
      GLctx.uniform1i(GL.uniforms[location], v0);
    }

  function _emscripten_glStencilMask(x0) { GLctx['stencilMask'](x0) }

  function _emscripten_glStencilFuncSeparate(x0, x1, x2, x3) { GLctx['stencilFuncSeparate'](x0, x1, x2, x3) }

   
  Module["_i64Subtract"] = _i64Subtract;

   
  Module["_i64Add"] = _i64Add;

  function _emscripten_set_touchend_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, 23, "touchend");
      return 0;
    }

  function _glUseProgram(program) {
      GLctx.useProgram(program ? GL.programs[program] : null);
    }

  function _emscripten_glDisableVertexAttribArray(index) {
      GLctx.disableVertexAttribArray(index);
    }

  function _emscripten_glVertexAttrib1f(x0, x1) { GLctx['vertexAttrib1f'](x0, x1) }

  function _emscripten_glFinish() { GLctx['finish']() }

  function _glDrawArrays(mode, first, count) {
  
      GLctx.drawArrays(mode, first, count);
  
    }

  function _emscripten_glDepthFunc(x0) { GLctx['depthFunc'](x0) }

  function _emscripten_get_num_gamepads() {
      // Polling gamepads generates garbage, so don't do it when we know there are no gamepads connected.
      if (!JSEvents.numGamepadsConnected) return 0;
  
      __emscripten_sample_gamepad_data();
      if (!JSEvents.lastGamepadState) return -1;
      return JSEvents.lastGamepadState.length;
    }

  function _emscripten_glUniform4iv(location, count, value) {
  
  
      GLctx.uniform4iv(GL.uniforms[location], HEAP32.subarray((value)>>2,(value+count*16)>>2));
    }

  function _glClear(x0) { GLctx['clear'](x0) }

  function _emscripten_glLoadIdentity(){ throw 'Legacy GL function (glLoadIdentity) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; }

  function _emscripten_glUniform3fv(location, count, value) {
  
  
      var view;
      if (3*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[3*count-1];
        for (var i = 0; i < 3*count; i += 3) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*12)>>2);
      }
      GLctx.uniform3fv(GL.uniforms[location], view);
    }

  function _emscripten_glIsTexture(texture) {
      var texture = GL.textures[texture];
      if (!texture) return 0;
      return GLctx.isTexture(texture);
    }

  function _glEnableVertexAttribArray(index) {
      GLctx.enableVertexAttribArray(index);
    }

  function _emscripten_glAttachShader(program, shader) {
      GLctx.attachShader(GL.programs[program],
                              GL.shaders[shader]);
    }

  function _glUniform4f(location, v0, v1, v2, v3) {
      GLctx.uniform4f(GL.uniforms[location], v0, v1, v2, v3);
    }

  function _emscripten_request_pointerlock(target, deferUntilInEventHandler) {
      if (!target) target = '#canvas';
      target = JSEvents.findEventTarget(target);
      if (!target) return -4;
      if (!target.requestPointerLock && !target.mozRequestPointerLock && !target.webkitRequestPointerLock && !target.msRequestPointerLock) {
        return -1;
      }
  
      var canPerformRequests = JSEvents.canPerformEventHandlerRequests();
  
      // Queue this function call if we're not currently in an event handler and the user saw it appropriate to do so.
      if (!canPerformRequests) {
        if (deferUntilInEventHandler) {
          JSEvents.deferCall(JSEvents.requestPointerLock, 2 /* priority below fullscreen */, [target]);
          return 1;
        } else {
          return -2;
        }
      }
  
      return JSEvents.requestPointerLock(target);
    }

  function _emscripten_glVertexAttrib2f(x0, x1, x2) { GLctx['vertexAttrib2f'](x0, x1, x2) }

  function _glfwCreateWindow(width, height, title, monitor, share) {
      return GLFW.createWindow(width, height, title, monitor, share);
    }

  function _glfwDefaultWindowHints() {
      GLFW.hints = GLFW.defaultHints;
    }

  function _emscripten_glClearStencil(x0) { GLctx['clearStencil'](x0) }

  function _emscripten_glDetachShader(program, shader) {
      GLctx.detachShader(GL.programs[program],
                              GL.shaders[shader]);
    }

  function _emscripten_glDeleteVertexArrays(n, vaos) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((vaos)+(i*4))>>2)];
        GLctx['deleteVertexArray'](GL.vaos[id]);
        GL.vaos[id] = null;
      }
    }

  function _glfwInit() {
      if (GLFW.windows) return 1; // GL_TRUE
  
      GLFW.initialTime = GLFW.getTime();
      GLFW.hints = GLFW.defaultHints;
      GLFW.windows = new Array()
      GLFW.active = null;
  
      window.addEventListener("keydown", GLFW.onKeydown, true);
      window.addEventListener("keypress", GLFW.onKeyPress, true);
      window.addEventListener("keyup", GLFW.onKeyup, true);
      Module["canvas"].addEventListener("mousemove", GLFW.onMousemove, true);
      Module["canvas"].addEventListener("mousedown", GLFW.onMouseButtonDown, true);
      Module["canvas"].addEventListener("mouseup", GLFW.onMouseButtonUp, true);
      Module["canvas"].addEventListener('wheel', GLFW.onMouseWheel, true);
      Module["canvas"].addEventListener('mousewheel', GLFW.onMouseWheel, true);
      Module["canvas"].addEventListener('mouseenter', GLFW.onMouseenter, true);
      Module["canvas"].addEventListener('mouseleave', GLFW.onMouseleave, true);
  
      Browser.resizeListeners.push(function(width, height) {
         GLFW.onCanvasResize(width, height);
      });
      return 1; // GL_TRUE
    }

  function _emscripten_glGetTexParameteriv(target, pname, params) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)]=GLctx.getTexParameter(target, pname);
    }

  function _glfwSwapBuffers(winid) {
      GLFW.swapBuffers(winid);
    }

  function _emscripten_glGenerateMipmap(x0) { GLctx['generateMipmap'](x0) }

  function _emscripten_glCullFace(x0) { GLctx['cullFace'](x0) }

  function _emscripten_glUniform4f(location, v0, v1, v2, v3) {
      GLctx.uniform4f(GL.uniforms[location], v0, v1, v2, v3);
    }

  function _glDisableVertexAttribArray(index) {
      GLctx.disableVertexAttribArray(index);
    }

  function _emscripten_glUseProgram(program) {
      GLctx.useProgram(program ? GL.programs[program] : null);
    }

  function _emscripten_glHint(x0, x1) { GLctx['hint'](x0, x1) }

  function _emscripten_glUniform2fv(location, count, value) {
  
  
      var view;
      if (2*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[2*count-1];
        for (var i = 0; i < 2*count; i += 2) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*8)>>2);
      }
      GLctx.uniform2fv(GL.uniforms[location], view);
    }

  function _glfwSwapInterval(interval) {
      interval = Math.abs(interval); // GLFW uses negative values to enable GLX_EXT_swap_control_tear, which we don't have, so just treat negative and positive the same.
      if (interval == 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 0);
      else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, interval);
    }

  function _glGetShaderInfoLog(shader, maxLength, length, infoLog) {
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      if (log === null) log = '(unknown error)';
      if (maxLength > 0 && infoLog) {
        var numBytesWrittenExclNull = stringToUTF8(log, infoLog, maxLength);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
    }

  function _emscripten_glMatrixMode(){ throw 'Legacy GL function (glMatrixMode) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; }

  function _abort() {
      Module['abort']();
    }

  function _emscripten_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer) {
      GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                         GL.renderbuffers[renderbuffer]);
    }

  function _emscripten_glDeleteFramebuffers(n, framebuffers) {
      for (var i = 0; i < n; ++i) {
        var id = HEAP32[(((framebuffers)+(i*4))>>2)];
        var framebuffer = GL.framebuffers[id];
        if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
        GLctx.deleteFramebuffer(framebuffer);
        framebuffer.name = 0;
        GL.framebuffers[id] = null;
      }
    }

  function _emscripten_glIsBuffer(buffer) {
      var b = GL.buffers[buffer];
      if (!b) return 0;
      return GLctx.isBuffer(b);
    }

  function _emscripten_glUniform2iv(location, count, value) {
  
  
      GLctx.uniform2iv(GL.uniforms[location], HEAP32.subarray((value)>>2,(value+count*8)>>2));
    }

  function _emscripten_glVertexAttrib1fv(index, v) {
  
      GLctx.vertexAttrib1f(index, HEAPF32[v>>2]);
    }

  function _glEnable(x0) { GLctx['enable'](x0) }

  
  
  function emscriptenWebGLComputeImageSize(width, height, sizePerPixel, alignment) {
      function roundedToNextMultipleOf(x, y) {
        return Math.floor((x + y - 1) / y) * y
      }
      var plainRowSize = width * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, alignment);
      return (height <= 0) ? 0 :
               ((height - 1) * alignedRowSize + plainRowSize);
    }function emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) {
      var sizePerPixel;
      var numChannels;
      switch(format) {
        case 0x1906 /* GL_ALPHA */:
        case 0x1909 /* GL_LUMINANCE */:
        case 0x1902 /* GL_DEPTH_COMPONENT */:
          numChannels = 1;
          break;
        case 0x190A /* GL_LUMINANCE_ALPHA */:
          numChannels = 2;
          break;
        case 0x1907 /* GL_RGB */:
        case 0x8C40 /* GL_SRGB_EXT */:
          numChannels = 3;
          break;
        case 0x1908 /* GL_RGBA */:
        case 0x8C42 /* GL_SRGB_ALPHA_EXT */:
          numChannels = 4;
          break;
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
          return null;
      }
      switch (type) {
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          sizePerPixel = numChannels*1;
          break;
        case 0x1403 /* GL_UNSIGNED_SHORT */:
        case 0x8D61 /* GL_HALF_FLOAT_OES */:
          sizePerPixel = numChannels*2;
          break;
        case 0x1405 /* GL_UNSIGNED_INT */:
        case 0x1406 /* GL_FLOAT */:
          sizePerPixel = numChannels*4;
          break;
        case 0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */:
          sizePerPixel = 4;
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          break;
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
          return null;
      }
      var bytes = emscriptenWebGLComputeImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      switch(type) {
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          return HEAPU8.subarray((pixels),(pixels+bytes));
        case 0x1406 /* GL_FLOAT */:
          return HEAPF32.subarray((pixels)>>2,(pixels+bytes)>>2);
        case 0x1405 /* GL_UNSIGNED_INT */:
        case 0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */:
          return HEAPU32.subarray((pixels)>>2,(pixels+bytes)>>2);
        case 0x1403 /* GL_UNSIGNED_SHORT */:
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
        case 0x8D61 /* GL_HALF_FLOAT_OES */:
          return HEAPU16.subarray((pixels)>>1,(pixels+bytes)>>1);
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
          return null;
      }
    }function _emscripten_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) {
      var pixelData = null;
      if (pixels) pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0);
      GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
    }

  function _emscripten_glPolygonOffset(x0, x1) { GLctx['polygonOffset'](x0, x1) }

  var _emscripten_asm_const_int=true;

  function _emscripten_glUniform2f(location, v0, v1) {
      GLctx.uniform2f(GL.uniforms[location], v0, v1);
    }

  function _glGetAttribLocation(program, name) {
      program = GL.programs[program];
      name = Pointer_stringify(name);
      return GLctx.getAttribLocation(program, name);
    }

  function _glfwWindowHint(target, hint) {
      GLFW.hints[target] = hint;
    }

  function _emscripten_glUniform2i(location, v0, v1) {
      GLctx.uniform2i(GL.uniforms[location], v0, v1);
    }

  function _glBlendFunc(x0, x1) { GLctx['blendFunc'](x0, x1) }

  function _glCreateProgram() {
      var id = GL.getNewId(GL.programs);
      var program = GLctx.createProgram();
      program.name = id;
      GL.programs[id] = program;
      return id;
    }

  function _emscripten_glDeleteRenderbuffers(n, renderbuffers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((renderbuffers)+(i*4))>>2)];
        var renderbuffer = GL.renderbuffers[id];
        if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
        GLctx.deleteRenderbuffer(renderbuffer);
        renderbuffer.name = 0;
        GL.renderbuffers[id] = null;
      }
    }

  function _emscripten_glGetBufferParameteriv(target, value, data) {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((data)>>2)]=GLctx.getBufferParameter(target, value);
    }

  
  function emscriptenWebGLGetUniform(program, location, params, type) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      var data = GLctx.getUniform(GL.programs[program], GL.uniforms[location]);
      if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 'Integer': HEAP32[((params)>>2)]=data; break;
          case 'Float': HEAPF32[((params)>>2)]=data; break;
          default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 'Integer': HEAP32[(((params)+(i))>>2)]=data[i]; break;
            case 'Float': HEAPF32[(((params)+(i))>>2)]=data[i]; break;
            default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
          }
        }
      }
    }function _emscripten_glGetUniformiv(program, location, params) {
      emscriptenWebGLGetUniform(program, location, params, 'Integer');
    }

  function _emscripten_glDepthMask(flag) {
      GLctx.depthMask(!!flag);
    }


  function _emscripten_glDepthRangef(x0, x1) { GLctx['depthRange'](x0, x1) }

  function _emscripten_glDepthRange(x0, x1) { GLctx['depthRange'](x0, x1) }

  function _emscripten_set_fullscreenchange_callback(target, userData, useCapture, callbackfunc) {
      if (typeof JSEvents.fullscreenEnabled() === 'undefined') return -1;
      if (!target) target = document;
      else {
        target = JSEvents.findEventTarget(target);
        if (!target) return -4;
      }
      JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "fullscreenchange");
      JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "mozfullscreenchange");
      JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "webkitfullscreenchange");
      JSEvents.registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "msfullscreenchange");
      return 0;
    }

  function _emscripten_glGetShaderPrecisionFormat(shaderType, precisionType, range, precision) {
      var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
      HEAP32[((range)>>2)]=result.rangeMin;
      HEAP32[(((range)+(4))>>2)]=result.rangeMax;
      HEAP32[((precision)>>2)]=result.precision;
    }

  function _emscripten_glUniform1fv(location, count, value) {
  
  
      var view;
      if (count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[count-1];
        for (var i = 0; i < count; ++i) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*4)>>2);
      }
      GLctx.uniform1fv(GL.uniforms[location], view);
    }

  function _glDeleteBuffers(n, buffers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((buffers)+(i*4))>>2)];
        var buffer = GL.buffers[id];
  
        // From spec: "glDeleteBuffers silently ignores 0's and names that do not
        // correspond to existing buffer objects."
        if (!buffer) continue;
  
        GLctx.deleteBuffer(buffer);
        buffer.name = 0;
        GL.buffers[id] = null;
  
        if (id == GL.currArrayBuffer) GL.currArrayBuffer = 0;
        if (id == GL.currElementArrayBuffer) GL.currElementArrayBuffer = 0;
      }
    }

  function _emscripten_set_gamepaddisconnected_callback(userData, useCapture, callbackfunc) {
      if (!navigator.getGamepads && !navigator.webkitGetGamepads) return -1;
      JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, 27, "gamepaddisconnected");
      return 0;
   }

  function _emscripten_glBindProgramARB() {
  Module['printErr']('missing function: emscripten_glBindProgramARB'); abort(-1);
  }

  function _emscripten_glBindTexture(target, texture) {
      GLctx.bindTexture(target, texture ? GL.textures[texture] : null);
    }

  function _emscripten_glCheckFramebufferStatus(x0) { return GLctx['checkFramebufferStatus'](x0) }

  function _emscripten_glDeleteProgram(id) {
      if (!id) return;
      var program = GL.programs[id];
      if (!program) { // glDeleteProgram actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteProgram(program);
      program.name = 0;
      GL.programs[id] = null;
      GL.programInfos[id] = null;
    }

  function _emscripten_glDisable(x0) { GLctx['disable'](x0) }

  function _emscripten_glVertexAttrib3fv(index, v) {
  
      GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
    }

  function _glClearColor(x0, x1, x2, x3) { GLctx['clearColor'](x0, x1, x2, x3) }

  function _emscripten_glGetActiveAttrib(program, index, bufSize, length, size, type, name) {
      program = GL.programs[program];
      var info = GLctx.getActiveAttrib(program, index);
      if (!info) return; // If an error occurs, nothing will be written to length, size and type and name.
  
      if (bufSize > 0 && name) {
        var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
  
      if (size) HEAP32[((size)>>2)]=info.size;
      if (type) HEAP32[((type)>>2)]=info.type;
    }

  function _emscripten_glIsFramebuffer(framebuffer) {
      var fb = GL.framebuffers[framebuffer];
      if (!fb) return 0;
      return GLctx.isFramebuffer(fb);
    }

  function _emscripten_glLineWidth(x0) { GLctx['lineWidth'](x0) }

  function _glfwGetCursorPos(winid, x, y) {
      GLFW.getCursorPos(winid, x, y);
    }

  function _emscripten_glGetString(name_) {
      if (GL.stringCache[name_]) return GL.stringCache[name_];
      var ret; 
      switch(name_) {
        case 0x1F00 /* GL_VENDOR */:
        case 0x1F01 /* GL_RENDERER */:
        case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
        case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
          ret = allocate(intArrayFromString(GLctx.getParameter(name_)), 'i8', ALLOC_NORMAL);
          break;
        case 0x1F02 /* GL_VERSION */:
          var glVersion = GLctx.getParameter(GLctx.VERSION);
          // return GLES version string corresponding to the version of the WebGL context
          {
            glVersion = 'OpenGL ES 2.0 (' + glVersion + ')';
          }
          ret = allocate(intArrayFromString(glVersion), 'i8', ALLOC_NORMAL);
          break;
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = GLctx.getSupportedExtensions();
          var gl_exts = [];
          for (var i = 0; i < exts.length; ++i) {
            gl_exts.push(exts[i]);
            gl_exts.push("GL_" + exts[i]);
          }
          ret = allocate(intArrayFromString(gl_exts.join(' ')), 'i8', ALLOC_NORMAL);
          break;
        case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
          var glslVersion = GLctx.getParameter(GLctx.SHADING_LANGUAGE_VERSION);
          // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
          var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
          var ver_num = glslVersion.match(ver_re);
          if (ver_num !== null) {
            if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
            glslVersion = 'OpenGL ES GLSL ES ' + ver_num[1] + ' (' + glslVersion + ')';
          }
          ret = allocate(intArrayFromString(glslVersion), 'i8', ALLOC_NORMAL);
          break;
        default:
          GL.recordError(0x0500/*GL_INVALID_ENUM*/);
          return 0;
      }
      GL.stringCache[name_] = ret;
      return ret;
    }

  function _emscripten_glGetAttribLocation(program, name) {
      program = GL.programs[program];
      name = Pointer_stringify(name);
      return GLctx.getAttribLocation(program, name);
    }

  function _emscripten_glRotatef() {
  Module['printErr']('missing function: emscripten_glRotatef'); abort(-1);
  }

  
  function emscriptenWebGLGet(name_, p, type) {
      // Guard against user passing a null pointer.
      // Note that GLES2 spec does not say anything about how passing a null pointer should be treated.
      // Testing on desktop core GL 3, the application crashes on glGetIntegerv to a null pointer, but
      // better to report an error instead of doing anything random.
      if (!p) {
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = undefined;
      switch(name_) { // Handle a few trivial GLES values
        case 0x8DFA: // GL_SHADER_COMPILER
          ret = 1;
          break;
        case 0x8DF8: // GL_SHADER_BINARY_FORMATS
          if (type !== 'Integer' && type !== 'Integer64') {
            GL.recordError(0x0500); // GL_INVALID_ENUM
          }
          return; // Do not write anything to the out pointer, since no binary formats are supported.
        case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
          ret = 0;
          break;
        case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
          // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be queried for length),
          // so implement it ourselves to allow C++ GLES2 code get the length.
          var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
          ret = formats.length;
          break;
      }
  
      if (ret === undefined) {
        var result = GLctx.getParameter(name_);
        switch (typeof(result)) {
          case "number":
            ret = result;
            break;
          case "boolean":
            ret = result ? 1 : 0;
            break;
          case "string":
            GL.recordError(0x0500); // GL_INVALID_ENUM
            return;
          case "object":
            if (result === null) {
              // null is a valid result for some (e.g., which buffer is bound - perhaps nothing is bound), but otherwise
              // can mean an invalid name_, which we need to report as an error
              switch(name_) {
                case 0x8894: // ARRAY_BUFFER_BINDING
                case 0x8B8D: // CURRENT_PROGRAM
                case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
                case 0x8CA6: // FRAMEBUFFER_BINDING
                case 0x8CA7: // RENDERBUFFER_BINDING
                case 0x8069: // TEXTURE_BINDING_2D
                case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                  ret = 0;
                  break;
                }
                default: {
                  GL.recordError(0x0500); // GL_INVALID_ENUM
                  return;
                }
              }
            } else if (result instanceof Float32Array ||
                       result instanceof Uint32Array ||
                       result instanceof Int32Array ||
                       result instanceof Array) {
              for (var i = 0; i < result.length; ++i) {
                switch (type) {
                  case 'Integer': HEAP32[(((p)+(i*4))>>2)]=result[i];   break;
                  case 'Float':   HEAPF32[(((p)+(i*4))>>2)]=result[i]; break;
                  case 'Boolean': HEAP8[(((p)+(i))>>0)]=result[i] ? 1 : 0;    break;
                  default: throw 'internal glGet error, bad type: ' + type;
                }
              }
              return;
            } else if (result instanceof WebGLBuffer ||
                       result instanceof WebGLProgram ||
                       result instanceof WebGLFramebuffer ||
                       result instanceof WebGLRenderbuffer ||
                       result instanceof WebGLTexture) {
              ret = result.name | 0;
            } else {
              GL.recordError(0x0500); // GL_INVALID_ENUM
              return;
            }
            break;
          default:
            GL.recordError(0x0500); // GL_INVALID_ENUM
            return;
        }
      }
  
      switch (type) {
        case 'Integer64': (tempI64 = [ret>>>0,(tempDouble=ret,(+(Math_abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math_min((+(Math_floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math_ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[((p)>>2)]=tempI64[0],HEAP32[(((p)+(4))>>2)]=tempI64[1]);    break;
        case 'Integer': HEAP32[((p)>>2)]=ret;    break;
        case 'Float':   HEAPF32[((p)>>2)]=ret;  break;
        case 'Boolean': HEAP8[((p)>>0)]=ret ? 1 : 0; break;
        default: throw 'internal glGet error, bad type: ' + type;
      }
    }function _emscripten_glGetIntegerv(name_, p) {
      emscriptenWebGLGet(name_, p, 'Integer');
    }

  function _emscripten_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params) {
      var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
      HEAP32[((params)>>2)]=result;
    }

  function _llvm_stackrestore(p) {
      var self = _llvm_stacksave;
      var ret = self.LLVM_SAVEDSTACKS[p];
      self.LLVM_SAVEDSTACKS.splice(p, 1);
      Runtime.stackRestore(ret);
    }

  function _glfwSetWindowShouldClose(winid, value) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.shouldClose = value;
    }

  function _emscripten_glClientActiveTexture() {
  Module['printErr']('missing function: emscripten_glClientActiveTexture'); abort(-1);
  }

  function _glGenBuffers(n, buffers) {
      for (var i = 0; i < n; i++) {
        var buffer = GLctx.createBuffer();
        if (!buffer) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
          while(i < n) HEAP32[(((buffers)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.buffers);
        buffer.name = id;
        GL.buffers[id] = buffer;
        HEAP32[(((buffers)+(i*4))>>2)]=id;
      }
    }

  
  function _emscripten_memcpy_big(dest, src, num) {
      HEAPU8.set(HEAPU8.subarray(src, src+num), dest);
      return dest;
    } 
  Module["_memcpy"] = _memcpy;

  function _emscripten_glGetShaderInfoLog(shader, maxLength, length, infoLog) {
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      if (log === null) log = '(unknown error)';
      if (maxLength > 0 && infoLog) {
        var numBytesWrittenExclNull = stringToUTF8(log, infoLog, maxLength);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
    }

  function _glfwGetTime() {
      return GLFW.getTime() - GLFW.initialTime;
    }

  function _emscripten_glGetRenderbufferParameteriv(target, pname, params) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)]=GLctx.getRenderbufferParameter(target, pname);
    }

  function _emscripten_glStencilOpSeparate(x0, x1, x2, x3) { GLctx['stencilOpSeparate'](x0, x1, x2, x3) }

  function _emscripten_glReadPixels(x, y, width, height, format, type, pixels) {
      var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, format);
      if (!pixelData) {
        GL.recordError(0x0500/*GL_INVALID_ENUM*/);
        return;
      }
      GLctx.readPixels(x, y, width, height, format, type, pixelData);
    }

  function _emscripten_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
      GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, data ? HEAPU8.subarray((data),(data+imageSize)) : null);
    }

  function _emscripten_glGetError() {
      // First return any GL error generated by the emscripten library_gl.js interop layer.
      if (GL.lastError) {
        var error = GL.lastError;
        GL.lastError = 0/*GL_NO_ERROR*/;
        return error;
      } else { // If there were none, return the GL error from the browser GL context.
        return GLctx.getError();
      }
    }

  function _emscripten_glFramebufferTexture2D(target, attachment, textarget, texture, level) {
      GLctx.framebufferTexture2D(target, attachment, textarget,
                                      GL.textures[texture], level);
    }

  function _emscripten_glIsEnabled(x0) { return GLctx['isEnabled'](x0) }

  function _glClearDepthf(x0) { GLctx['clearDepth'](x0) }

   
  Module["_memmove"] = _memmove;

  function _glGenTextures(n, textures) {
      for (var i = 0; i < n; i++) {
        var texture = GLctx.createTexture();
        if (!texture) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */); // GLES + EGL specs don't specify what should happen here, so best to issue an error and create IDs with 0.
          while(i < n) HEAP32[(((textures)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.textures);
        texture.name = id;
        GL.textures[id] = texture;
        HEAP32[(((textures)+(i*4))>>2)]=id;
      }
    }

  function _emscripten_glVertexAttrib4f(x0, x1, x2, x3, x4) { GLctx['vertexAttrib4f'](x0, x1, x2, x3, x4) }

  function _glDepthFunc(x0) { GLctx['depthFunc'](x0) }

  
  
  var cttz_i8 = allocate([8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0], "i8", ALLOC_STATIC); 
  Module["_llvm_cttz_i32"] = _llvm_cttz_i32; 
  Module["___udivmoddi4"] = ___udivmoddi4; 
  Module["___uremdi3"] = ___uremdi3;

  function _emscripten_glClearDepthf(x0) { GLctx['clearDepth'](x0) }

  function _emscripten_glClear(x0) { GLctx['clear'](x0) }

  function _emscripten_glBindBuffer(target, buffer) {
      var bufferObj = buffer ? GL.buffers[buffer] : null;
  
  
      GLctx.bindBuffer(target, bufferObj);
    }

  function _emscripten_glGetUniformfv(program, location, params) {
      emscriptenWebGLGetUniform(program, location, params, 'Float');
    }

  function _glGetProgramiv(program, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
  
      if (program >= GL.counter) {
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
  
      var ptable = GL.programInfos[program];
      if (!ptable) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
        return;
      }
  
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getProgramInfoLog(GL.programs[program]);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)]=log.length + 1;
      } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
        HEAP32[((p)>>2)]=ptable.maxUniformLength;
      } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
        if (ptable.maxAttributeLength == -1) {
          var program = GL.programs[program];
          var numAttribs = GLctx.getProgramParameter(program, GLctx.ACTIVE_ATTRIBUTES);
          ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
          for (var i = 0; i < numAttribs; ++i) {
            var activeAttrib = GLctx.getActiveAttrib(program, i);
            ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
          }
        }
        HEAP32[((p)>>2)]=ptable.maxAttributeLength;
      } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
        if (ptable.maxUniformBlockNameLength == -1) {
          var program = GL.programs[program];
          var numBlocks = GLctx.getProgramParameter(program, GLctx.ACTIVE_UNIFORM_BLOCKS);
          ptable.maxUniformBlockNameLength = 0;
          for (var i = 0; i < numBlocks; ++i) {
            var activeBlockName = GLctx.getActiveUniformBlockName(program, i);
            ptable.maxUniformBlockNameLength = Math.max(ptable.maxUniformBlockNameLength, activeBlockName.length+1);
          }
        }
        HEAP32[((p)>>2)]=ptable.maxUniformBlockNameLength;
      } else {
        HEAP32[((p)>>2)]=GLctx.getProgramParameter(GL.programs[program], pname);
      }
    }

  function _glVertexAttribPointer(index, size, type, normalized, stride, ptr) {
      GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
    }

  function _emscripten_exit_pointerlock() {
      // Make sure no queued up calls will fire after this.
      JSEvents.removeDeferredCalls(JSEvents.requestPointerLock);
  
      if (document.exitPointerLock) {
        document.exitPointerLock();
      } else if (document.msExitPointerLock) {
        document.msExitPointerLock();
      } else if (document.mozExitPointerLock) {
        document.mozExitPointerLock();
      } else if (document.webkitExitPointerLock) {
        document.webkitExitPointerLock();
      } else {
        return -1;
      }
      return 0;
    }

  function _glGetUniformLocation(program, name) {
      name = Pointer_stringify(name);
  
      var arrayOffset = 0;
      // If user passed an array accessor "[index]", parse the array index off the accessor.
      if (name.indexOf(']', name.length-1) !== -1) {
        var ls = name.lastIndexOf('[');
        var arrayIndex = name.slice(ls+1, -1);
        if (arrayIndex.length > 0) {
          arrayOffset = parseInt(arrayIndex);
          if (arrayOffset < 0) {
            return -1;
          }
        }
        name = name.slice(0, ls);
      }
  
      var ptable = GL.programInfos[program];
      if (!ptable) {
        return -1;
      }
      var utable = ptable.uniforms;
      var uniformInfo = utable[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
      if (uniformInfo && arrayOffset < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
        return uniformInfo[1]+arrayOffset;
      } else {
        return -1;
      }
    }

  function _emscripten_glGetAttachedShaders(program, maxCount, count, shaders) {
      var result = GLctx.getAttachedShaders(GL.programs[program]);
      var len = result.length;
      if (len > maxCount) {
        len = maxCount;
      }
      HEAP32[((count)>>2)]=len;
      for (var i = 0; i < len; ++i) {
        var id = GL.shaders.indexOf(result[i]);
        assert(id !== -1, 'shader not bound to local id');
        HEAP32[(((shaders)+(i*4))>>2)]=id;
      }
    }

  function _emscripten_glGenRenderbuffers(n, renderbuffers) {
      for (var i = 0; i < n; i++) {
        var renderbuffer = GLctx.createRenderbuffer();
        if (!renderbuffer) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
          while(i < n) HEAP32[(((renderbuffers)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.renderbuffers);
        renderbuffer.name = id;
        GL.renderbuffers[id] = renderbuffer;
        HEAP32[(((renderbuffers)+(i*4))>>2)]=id;
      }
    }

  function _emscripten_glFrontFace(x0) { GLctx['frontFace'](x0) }

  function _emscripten_glActiveTexture(x0) { GLctx['activeTexture'](x0) }

  function _emscripten_glUniform1iv(location, count, value) {
  
  
      GLctx.uniform1iv(GL.uniforms[location], HEAP32.subarray((value)>>2,(value+count*4)>>2));
    }

  function _emscripten_glTexCoordPointer() {
  Module['printErr']('missing function: emscripten_glTexCoordPointer'); abort(-1);
  }

  function _emscripten_glGetInfoLogARB() {
  Module['printErr']('missing function: emscripten_glGetInfoLogARB'); abort(-1);
  }

  
  function __exit(status) {
      // void _exit(int status);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html
      Module['exit'](status);
    }function _exit(status) {
      __exit(status);
    }

  function _emscripten_glRenderbufferStorage(x0, x1, x2, x3) { GLctx['renderbufferStorage'](x0, x1, x2, x3) }

  function _emscripten_glCopyTexSubImage2D(x0, x1, x2, x3, x4, x5, x6, x7) { GLctx['copyTexSubImage2D'](x0, x1, x2, x3, x4, x5, x6, x7) }

  function _glfwSetCursorPosCallback(winid, cbfun) {
      GLFW.setCursorPosCallback(winid, cbfun);
    }

  function _glBindAttribLocation(program, index, name) {
      name = Pointer_stringify(name);
      GLctx.bindAttribLocation(GL.programs[program], index, name);
    }

  function _emscripten_glShaderBinary() {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
    }

  function _emscripten_glIsProgram(program) {
      var program = GL.programs[program];
      if (!program) return 0;
      return GLctx.isProgram(program);
    }

  function _emscripten_glBlendColor(x0, x1, x2, x3) { GLctx['blendColor'](x0, x1, x2, x3) }

  function _emscripten_glGetShaderiv(shader, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)]=log.length + 1;
      } else {
        HEAP32[((p)>>2)]=GLctx.getShaderParameter(GL.shaders[shader], pname);
      }
    }

  function _emscripten_glUniformMatrix3fv(location, count, transpose, value) {
  
  
      var view;
      if (9*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[9*count-1];
        for (var i = 0; i < 9*count; i += 9) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
          view[i+4] = HEAPF32[(((value)+(4*i+16))>>2)];
          view[i+5] = HEAPF32[(((value)+(4*i+20))>>2)];
          view[i+6] = HEAPF32[(((value)+(4*i+24))>>2)];
          view[i+7] = HEAPF32[(((value)+(4*i+28))>>2)];
          view[i+8] = HEAPF32[(((value)+(4*i+32))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*36)>>2);
      }
      GLctx.uniformMatrix3fv(GL.uniforms[location], !!transpose, view);
    }

   
  Module["___udivdi3"] = ___udivdi3;

  function _emscripten_glUniform4fv(location, count, value) {
  
  
      var view;
      if (4*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[4*count-1];
        for (var i = 0; i < 4*count; i += 4) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*16)>>2);
      }
      GLctx.uniform4fv(GL.uniforms[location], view);
    }

  function _glBufferSubData(target, offset, size, data) {
      GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
    }

  function _emscripten_glGenFramebuffers(n, ids) {
      for (var i = 0; i < n; ++i) {
        var framebuffer = GLctx.createFramebuffer();
        if (!framebuffer) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
          while(i < n) HEAP32[(((ids)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.framebuffers);
        framebuffer.name = id;
        GL.framebuffers[id] = framebuffer;
        HEAP32[(((ids)+(i*4))>>2)]=id;
      }
    }

  function _glGetShaderiv(shader, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)]=log.length + 1;
      } else {
        HEAP32[((p)>>2)]=GLctx.getShaderParameter(GL.shaders[shader], pname);
      }
    }

  function _emscripten_glBlendEquationSeparate(x0, x1) { GLctx['blendEquationSeparate'](x0, x1) }

  function _glfwSetWindowIconifyCallback(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.windowIconifyFunc = cbfun;
    }

  function _emscripten_glDrawRangeElements() {
  Module['printErr']('missing function: emscripten_glDrawRangeElements'); abort(-1);
  }

  function _emscripten_glGenTextures(n, textures) {
      for (var i = 0; i < n; i++) {
        var texture = GLctx.createTexture();
        if (!texture) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */); // GLES + EGL specs don't specify what should happen here, so best to issue an error and create IDs with 0.
          while(i < n) HEAP32[(((textures)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.textures);
        texture.name = id;
        GL.textures[id] = texture;
        HEAP32[(((textures)+(i*4))>>2)]=id;
      }
    }

  function _emscripten_glVertexAttrib2fv(index, v) {
  
      GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
    }

  function _emscripten_glGetActiveUniform(program, index, bufSize, length, size, type, name) {
      program = GL.programs[program];
      var info = GLctx.getActiveUniform(program, index);
      if (!info) return; // If an error occurs, nothing will be written to length, size, type and name.
  
      if (bufSize > 0 && name) {
        var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
  
      if (size) HEAP32[((size)>>2)]=info.size;
      if (type) HEAP32[((type)>>2)]=info.type;
    }

   
  Module["_roundf"] = _roundf;

  function _emscripten_glDeleteObjectARB() {
  Module['printErr']('missing function: emscripten_glDeleteObjectARB'); abort(-1);
  }

  function _emscripten_set_touchmove_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerTouchEventCallback(target, userData, useCapture, callbackfunc, 24, "touchmove");
      return 0;
    }

  function _emscripten_glUniform1f(location, v0) {
      GLctx.uniform1f(GL.uniforms[location], v0);
    }

  function _emscripten_glVertexAttribPointer(index, size, type, normalized, stride, ptr) {
      GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
    }

  function _glShaderSource(shader, count, string, length) {
      var source = GL.getSource(shader, count, string, length);
  
  
      GLctx.shaderSource(GL.shaders[shader], source);
    }

  function _emscripten_glDrawArrays(mode, first, count) {
  
      GLctx.drawArrays(mode, first, count);
  
    }

  function _emscripten_glGenBuffers(n, buffers) {
      for (var i = 0; i < n; i++) {
        var buffer = GLctx.createBuffer();
        if (!buffer) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
          while(i < n) HEAP32[(((buffers)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.buffers);
        buffer.name = id;
        GL.buffers[id] = buffer;
        HEAP32[(((buffers)+(i*4))>>2)]=id;
      }
    }

  function _emscripten_glClearDepth(x0) { GLctx['clearDepth'](x0) }

  function _emscripten_set_keypress_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerKeyEventCallback(target, userData, useCapture, callbackfunc, 1, "keypress");
      return 0;
    }

  function _glfwSetCharCallback(winid, cbfun) {
      GLFW.setCharCallback(winid, cbfun);
    }

  function _emscripten_glGetUniformLocation(program, name) {
      name = Pointer_stringify(name);
  
      var arrayOffset = 0;
      // If user passed an array accessor "[index]", parse the array index off the accessor.
      if (name.indexOf(']', name.length-1) !== -1) {
        var ls = name.lastIndexOf('[');
        var arrayIndex = name.slice(ls+1, -1);
        if (arrayIndex.length > 0) {
          arrayOffset = parseInt(arrayIndex);
          if (arrayOffset < 0) {
            return -1;
          }
        }
        name = name.slice(0, ls);
      }
  
      var ptable = GL.programInfos[program];
      if (!ptable) {
        return -1;
      }
      var utable = ptable.uniforms;
      var uniformInfo = utable[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
      if (uniformInfo && arrayOffset < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
        return uniformInfo[1]+arrayOffset;
      } else {
        return -1;
      }
    }

  function _glBindBuffer(target, buffer) {
      var bufferObj = buffer ? GL.buffers[buffer] : null;
  
  
      GLctx.bindBuffer(target, bufferObj);
    }

  function _emscripten_glVertexAttrib4fv(index, v) {
  
      GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
    }

  function _emscripten_glScissor(x0, x1, x2, x3) { GLctx['scissor'](x0, x1, x2, x3) }

  function _glfwSetCursorEnterCallback(winid, cbfun) {
      var win = GLFW.WindowFromId(winid);
      if (!win) return;
      win.cursorEnterFunc = cbfun;
    }

   
  Module["_bitshift64Lshr"] = _bitshift64Lshr;

  function _glBufferData(target, size, data, usage) {
      if (!data) {
        GLctx.bufferData(target, size, usage);
      } else {
        GLctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
      }
    }

  function _emscripten_glIsShader(shader) {
      var s = GL.shaders[shader];
      if (!s) return 0;
      return GLctx.isShader(s);
    }

  function _emscripten_glDrawBuffers(n, bufs) {
  
      var bufArray = GL.tempFixedLengthArray[n];
      for (var i = 0; i < n; i++) {
        bufArray[i] = HEAP32[(((bufs)+(i*4))>>2)];
      }
  
      GLctx['drawBuffers'](bufArray);
    }

  function _glGetFloatv(name_, p) {
      emscriptenWebGLGet(name_, p, 'Float');
    }

  function _emscripten_glBindFramebuffer(target, framebuffer) {
      GLctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : null);
    }

  function _emscripten_glBlendEquation(x0) { GLctx['blendEquation'](x0) }

  function _emscripten_glBufferSubData(target, offset, size, data) {
      GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
    }

  function _emscripten_glBufferData(target, size, data, usage) {
      if (!data) {
        GLctx.bufferData(target, size, usage);
      } else {
        GLctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
      }
    }

  
  function ___setErrNo(value) {
      if (Module['___errno_location']) HEAP32[((Module['___errno_location']())>>2)]=value;
      else Module.printErr('failed to set errno from JS');
      return value;
    } 
  Module["_sbrk"] = _sbrk;

   
  Module["_bitshift64Shl"] = _bitshift64Shl;

  function _emscripten_glGetShaderSource(shader, bufSize, length, source) {
      var result = GLctx.getShaderSource(GL.shaders[shader]);
      if (!result) return; // If an error occurs, nothing will be written to length or source.
      if (bufSize > 0 && source) {
        var numBytesWrittenExclNull = stringToUTF8(result, source, bufSize);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
    }

   
  Module["_llvm_bswap_i32"] = _llvm_bswap_i32;

  function _emscripten_set_click_callback(target, userData, useCapture, callbackfunc) {
      JSEvents.registerMouseEventCallback(target, userData, useCapture, callbackfunc, 4, "click");
      return 0;
    }

  function _glfwSetKeyCallback(winid, cbfun) {
      GLFW.setKeyCallback(winid, cbfun);
    }

  function _emscripten_set_gamepadconnected_callback(userData, useCapture, callbackfunc) {
      if (!navigator.getGamepads && !navigator.webkitGetGamepads) return -1;
      JSEvents.registerGamepadEventCallback(window, userData, useCapture, callbackfunc, 26, "gamepadconnected");
      return 0;
    }

  function _emscripten_glGetFloatv(name_, p) {
      emscriptenWebGLGet(name_, p, 'Float');
    }

  function _glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels) {
  
      var pixelData = null;
      if (pixels) pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat);
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
    }

  function _glGetProgramInfoLog(program, maxLength, length, infoLog) {
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
      if (log === null) log = '(unknown error)';
  
      if (maxLength > 0 && infoLog) {
        var numBytesWrittenExclNull = stringToUTF8(log, infoLog, maxLength);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
    }

  function _emscripten_glVertexAttribDivisor(index, divisor) {
      GLctx['vertexAttribDivisor'](index, divisor);
    }

  function _emscripten_glDrawElementsInstanced(mode, count, type, indices, primcount) {
      GLctx['drawElementsInstanced'](mode, count, type, indices, primcount);
    }

  function _emscripten_glDrawElements(mode, count, type, indices) {
  
      GLctx.drawElements(mode, count, type, indices);
  
    }

  function _glfwSetMouseButtonCallback(winid, cbfun) {
      GLFW.setMouseButtonCallback(winid, cbfun);
    }

  function _emscripten_glCreateProgram() {
      var id = GL.getNewId(GL.programs);
      var program = GLctx.createProgram();
      program.name = id;
      GL.programs[id] = program;
      return id;
    }

  function _emscripten_glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data) {
      GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data ? HEAPU8.subarray((data),(data+imageSize)) : null);
    }

  function _emscripten_glClearColor(x0, x1, x2, x3) { GLctx['clearColor'](x0, x1, x2, x3) }

  function _emscripten_glBindVertexArray(vao) {
      GLctx['bindVertexArray'](GL.vaos[vao]);
    }

  function _emscripten_glLoadMatrixf() {
  Module['printErr']('missing function: emscripten_glLoadMatrixf'); abort(-1);
  }

  function _glDeleteShader(id) {
      if (!id) return;
      var shader = GL.shaders[id];
      if (!shader) { // glDeleteShader actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteShader(shader);
      GL.shaders[id] = null;
    }

  function _emscripten_glGetProgramiv(program, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it. 
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
  
      if (program >= GL.counter) {
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
        return;
      }
  
      var ptable = GL.programInfos[program];
      if (!ptable) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
        return;
      }
  
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getProgramInfoLog(GL.programs[program]);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)]=log.length + 1;
      } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
        HEAP32[((p)>>2)]=ptable.maxUniformLength;
      } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
        if (ptable.maxAttributeLength == -1) {
          var program = GL.programs[program];
          var numAttribs = GLctx.getProgramParameter(program, GLctx.ACTIVE_ATTRIBUTES);
          ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
          for (var i = 0; i < numAttribs; ++i) {
            var activeAttrib = GLctx.getActiveAttrib(program, i);
            ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
          }
        }
        HEAP32[((p)>>2)]=ptable.maxAttributeLength;
      } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
        if (ptable.maxUniformBlockNameLength == -1) {
          var program = GL.programs[program];
          var numBlocks = GLctx.getProgramParameter(program, GLctx.ACTIVE_UNIFORM_BLOCKS);
          ptable.maxUniformBlockNameLength = 0;
          for (var i = 0; i < numBlocks; ++i) {
            var activeBlockName = GLctx.getActiveUniformBlockName(program, i);
            ptable.maxUniformBlockNameLength = Math.max(ptable.maxUniformBlockNameLength, activeBlockName.length+1);
          }
        }
        HEAP32[((p)>>2)]=ptable.maxUniformBlockNameLength;
      } else {
        HEAP32[((p)>>2)]=GLctx.getProgramParameter(GL.programs[program], pname);
      }
    }

  function _emscripten_glGetProgramInfoLog(program, maxLength, length, infoLog) {
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
      if (log === null) log = '(unknown error)';
  
      if (maxLength > 0 && infoLog) {
        var numBytesWrittenExclNull = stringToUTF8(log, infoLog, maxLength);
        if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)]=0;
      }
    }

  function _emscripten_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels) {
  
      var pixelData = null;
      if (pixels) pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat);
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
    }

  function _glPixelStorei(pname, param) {
      if (pname == 0x0D05 /* GL_PACK_ALIGNMENT */) {
        GL.packAlignment = param;
      } else if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
        GL.unpackAlignment = param;
      }
      GLctx.pixelStorei(pname, param);
    }

  function ___unlock() {}

  function _emscripten_glColorPointer() {
  Module['printErr']('missing function: emscripten_glColorPointer'); abort(-1);
  }

  function _glViewport(x0, x1, x2, x3) { GLctx['viewport'](x0, x1, x2, x3) }

  function _glfwDestroyWindow(winid) {
      return GLFW.destroyWindow(winid);
    }

  function _emscripten_glFlush() { GLctx['flush']() }

  function _glfwSetErrorCallback(cbfun) {
      GLFW.errorFunc = cbfun;
    }

  function _emscripten_glCreateShader(shaderType) {
      var id = GL.getNewId(GL.shaders);
      GL.shaders[id] = GLctx.createShader(shaderType);
      return id;
    }

  function _glUniformMatrix4fv(location, count, transpose, value) {
  
  
      var view;
      if (16*count <= GL.MINI_TEMP_BUFFER_SIZE) {
        // avoid allocation when uploading few enough uniforms
        view = GL.miniTempBufferViews[16*count-1];
        for (var i = 0; i < 16*count; i += 16) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
          view[i+4] = HEAPF32[(((value)+(4*i+16))>>2)];
          view[i+5] = HEAPF32[(((value)+(4*i+20))>>2)];
          view[i+6] = HEAPF32[(((value)+(4*i+24))>>2)];
          view[i+7] = HEAPF32[(((value)+(4*i+28))>>2)];
          view[i+8] = HEAPF32[(((value)+(4*i+32))>>2)];
          view[i+9] = HEAPF32[(((value)+(4*i+36))>>2)];
          view[i+10] = HEAPF32[(((value)+(4*i+40))>>2)];
          view[i+11] = HEAPF32[(((value)+(4*i+44))>>2)];
          view[i+12] = HEAPF32[(((value)+(4*i+48))>>2)];
          view[i+13] = HEAPF32[(((value)+(4*i+52))>>2)];
          view[i+14] = HEAPF32[(((value)+(4*i+56))>>2)];
          view[i+15] = HEAPF32[(((value)+(4*i+60))>>2)];
        }
      } else {
        view = HEAPF32.subarray((value)>>2,(value+count*64)>>2);
      }
      GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, view);
    }

  function _emscripten_glValidateProgram(program) {
      GLctx.validateProgram(GL.programs[program]);
    }

  function _glTexParameteri(x0, x1, x2) { GLctx['texParameteri'](x0, x1, x2) }

  function _glFrontFace(x0) { GLctx['frontFace'](x0) }

  function _emscripten_glColorMask(red, green, blue, alpha) {
      GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
    }

  function _emscripten_glPixelStorei(pname, param) {
      if (pname == 0x0D05 /* GL_PACK_ALIGNMENT */) {
        GL.packAlignment = param;
      } else if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
        GL.unpackAlignment = param;
      }
      GLctx.pixelStorei(pname, param);
    }

  function _emscripten_glDeleteTextures(n, textures) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((textures)+(i*4))>>2)];
        var texture = GL.textures[id];
        if (!texture) continue; // GL spec: "glDeleteTextures silently ignores 0s and names that do not correspond to existing textures".
        GLctx.deleteTexture(texture);
        texture.name = 0;
        GL.textures[id] = null;
      }
    }

  function _emscripten_glGenVertexArrays(n, arrays) {
  
      for (var i = 0; i < n; i++) {
        var vao = GLctx['createVertexArray']();
        if (!vao) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
          while(i < n) HEAP32[(((arrays)+(i++*4))>>2)]=0;
          return;
        }
        var id = GL.getNewId(GL.vaos);
        vao.name = id;
        GL.vaos[id] = vao;
        HEAP32[(((arrays)+(i*4))>>2)]=id;
      }
    }

  function _time(ptr) {
      var ret = (Date.now()/1000)|0;
      if (ptr) {
        HEAP32[((ptr)>>2)]=ret;
      }
      return ret;
    }

  function _emscripten_glGetBooleanv(name_, p) {
      emscriptenWebGLGet(name_, p, 'Boolean');
    }

  function _emscripten_glCompileShader(shader) {
      GLctx.compileShader(GL.shaders[shader]);
    }
var GLctx; GL.init();
if (ENVIRONMENT_IS_NODE) {
    _emscripten_get_now = function _emscripten_get_now_actual() {
      var t = process['hrtime']();
      return t[0] * 1e3 + t[1] / 1e6;
    };
  } else if (typeof dateNow !== 'undefined') {
    _emscripten_get_now = dateNow;
  } else if (typeof self === 'object' && self['performance'] && typeof self['performance']['now'] === 'function') {
    _emscripten_get_now = function() { return self['performance']['now'](); };
  } else if (typeof performance === 'object' && typeof performance['now'] === 'function') {
    _emscripten_get_now = function() { return performance['now'](); };
  } else {
    _emscripten_get_now = Date.now;
  };
Module["requestFullScreen"] = function Module_requestFullScreen(lockPointer, resizeCanvas, vrDevice) { Module.printErr("Module.requestFullScreen is deprecated. Please call Module.requestFullscreen instead."); Module["requestFullScreen"] = Module["requestFullscreen"]; Browser.requestFullScreen(lockPointer, resizeCanvas, vrDevice) };
  Module["requestFullscreen"] = function Module_requestFullscreen(lockPointer, resizeCanvas, vrDevice) { Browser.requestFullscreen(lockPointer, resizeCanvas, vrDevice) };
  Module["requestAnimationFrame"] = function Module_requestAnimationFrame(func) { Browser.requestAnimationFrame(func) };
  Module["setCanvasSize"] = function Module_setCanvasSize(width, height, noUpdates) { Browser.setCanvasSize(width, height, noUpdates) };
  Module["pauseMainLoop"] = function Module_pauseMainLoop() { Browser.mainLoop.pause() };
  Module["resumeMainLoop"] = function Module_resumeMainLoop() { Browser.mainLoop.resume() };
  Module["getUserMedia"] = function Module_getUserMedia() { Browser.getUserMedia() }
  Module["createContext"] = function Module_createContext(canvas, useWebGL, setInModule, webGLContextAttributes) { return Browser.createContext(canvas, useWebGL, setInModule, webGLContextAttributes) };
JSEvents.staticInit();;
/* flush anything remaining in the buffer during shutdown */ __ATEXIT__.push(function() { var fflush = Module["_fflush"]; if (fflush) fflush(0); var printChar = ___syscall146.printChar; if (!printChar) return; var buffers = ___syscall146.buffers; if (buffers[1].length) printChar(1, 10); if (buffers[2].length) printChar(2, 10); });;
DYNAMICTOP_PTR = allocate(1, "i32", ALLOC_STATIC);

STACK_BASE = STACKTOP = Runtime.alignMemory(STATICTOP);

STACK_MAX = STACK_BASE + TOTAL_STACK;

DYNAMIC_BASE = Runtime.alignMemory(STACK_MAX);

HEAP32[DYNAMICTOP_PTR>>2] = DYNAMIC_BASE;

staticSealed = true; // seal the static portion of memory

assert(DYNAMIC_BASE < TOTAL_MEMORY, "TOTAL_MEMORY not big enough for stack");



function nullFunc_viiiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vd(x) { Module["printErr"]("Invalid function pointer called with signature 'vd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vid(x) { Module["printErr"]("Invalid function pointer called with signature 'vid'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vi(x) { Module["printErr"]("Invalid function pointer called with signature 'vi'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vii(x) { Module["printErr"]("Invalid function pointer called with signature 'vii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_ii(x) { Module["printErr"]("Invalid function pointer called with signature 'ii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viddd(x) { Module["printErr"]("Invalid function pointer called with signature 'viddd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vidd(x) { Module["printErr"]("Invalid function pointer called with signature 'vidd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_iiii(x) { Module["printErr"]("Invalid function pointer called with signature 'iiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viiiiiiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiiiiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viiiiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viii(x) { Module["printErr"]("Invalid function pointer called with signature 'viii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vidddd(x) { Module["printErr"]("Invalid function pointer called with signature 'vidddd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vdi(x) { Module["printErr"]("Invalid function pointer called with signature 'vdi'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viiiiiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiiiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viiiiiiiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiiiiiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_iii(x) { Module["printErr"]("Invalid function pointer called with signature 'iii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_i(x) { Module["printErr"]("Invalid function pointer called with signature 'i'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vdddddd(x) { Module["printErr"]("Invalid function pointer called with signature 'vdddddd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vdddd(x) { Module["printErr"]("Invalid function pointer called with signature 'vdddd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_vdd(x) { Module["printErr"]("Invalid function pointer called with signature 'vdd'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_v(x) { Module["printErr"]("Invalid function pointer called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viid(x) { Module["printErr"]("Invalid function pointer called with signature 'viid'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function nullFunc_viiii(x) { Module["printErr"]("Invalid function pointer called with signature 'viiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");  Module["printErr"]("Build with ASSERTIONS=2 for more info.");abort(x) }

function invoke_viiiii(index,a1,a2,a3,a4,a5) {
  try {
    Module["dynCall_viiiii"](index,a1,a2,a3,a4,a5);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vd(index,a1) {
  try {
    Module["dynCall_vd"](index,a1);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vid(index,a1,a2) {
  try {
    Module["dynCall_vid"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vi(index,a1) {
  try {
    Module["dynCall_vi"](index,a1);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vii(index,a1,a2) {
  try {
    Module["dynCall_vii"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_ii(index,a1) {
  try {
    return Module["dynCall_ii"](index,a1);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viddd(index,a1,a2,a3,a4) {
  try {
    Module["dynCall_viddd"](index,a1,a2,a3,a4);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vidd(index,a1,a2,a3) {
  try {
    Module["dynCall_vidd"](index,a1,a2,a3);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_iiii(index,a1,a2,a3) {
  try {
    return Module["dynCall_iiii"](index,a1,a2,a3);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8) {
  try {
    Module["dynCall_viiiiiiii"](index,a1,a2,a3,a4,a5,a6,a7,a8);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viiiiii(index,a1,a2,a3,a4,a5,a6) {
  try {
    Module["dynCall_viiiiii"](index,a1,a2,a3,a4,a5,a6);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viii(index,a1,a2,a3) {
  try {
    Module["dynCall_viii"](index,a1,a2,a3);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vidddd(index,a1,a2,a3,a4,a5) {
  try {
    Module["dynCall_vidddd"](index,a1,a2,a3,a4,a5);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vdi(index,a1,a2) {
  try {
    Module["dynCall_vdi"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  try {
    Module["dynCall_viiiiiii"](index,a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  try {
    Module["dynCall_viiiiiiiii"](index,a1,a2,a3,a4,a5,a6,a7,a8,a9);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_iii(index,a1,a2) {
  try {
    return Module["dynCall_iii"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_i(index) {
  try {
    return Module["dynCall_i"](index);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vdddddd(index,a1,a2,a3,a4,a5,a6) {
  try {
    Module["dynCall_vdddddd"](index,a1,a2,a3,a4,a5,a6);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vdddd(index,a1,a2,a3,a4) {
  try {
    Module["dynCall_vdddd"](index,a1,a2,a3,a4);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_vdd(index,a1,a2) {
  try {
    Module["dynCall_vdd"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_v(index) {
  try {
    Module["dynCall_v"](index);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viid(index,a1,a2,a3) {
  try {
    Module["dynCall_viid"](index,a1,a2,a3);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

function invoke_viiii(index,a1,a2,a3,a4) {
  try {
    Module["dynCall_viiii"](index,a1,a2,a3,a4);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    Module["setThrew"](1, 0);
  }
}

Module.asmGlobalArg = { "Math": Math, "Int8Array": Int8Array, "Int16Array": Int16Array, "Int32Array": Int32Array, "Uint8Array": Uint8Array, "Uint16Array": Uint16Array, "Uint32Array": Uint32Array, "Float32Array": Float32Array, "Float64Array": Float64Array, "NaN": NaN, "Infinity": Infinity };

Module.asmLibraryArg = { "abort": abort, "assert": assert, "enlargeMemory": enlargeMemory, "getTotalMemory": getTotalMemory, "abortOnCannotGrowMemory": abortOnCannotGrowMemory, "abortStackOverflow": abortStackOverflow, "nullFunc_viiiii": nullFunc_viiiii, "nullFunc_vd": nullFunc_vd, "nullFunc_vid": nullFunc_vid, "nullFunc_vi": nullFunc_vi, "nullFunc_vii": nullFunc_vii, "nullFunc_ii": nullFunc_ii, "nullFunc_viddd": nullFunc_viddd, "nullFunc_vidd": nullFunc_vidd, "nullFunc_iiii": nullFunc_iiii, "nullFunc_viiiiiiii": nullFunc_viiiiiiii, "nullFunc_viiiiii": nullFunc_viiiiii, "nullFunc_viii": nullFunc_viii, "nullFunc_vidddd": nullFunc_vidddd, "nullFunc_vdi": nullFunc_vdi, "nullFunc_viiiiiii": nullFunc_viiiiiii, "nullFunc_viiiiiiiii": nullFunc_viiiiiiiii, "nullFunc_iii": nullFunc_iii, "nullFunc_i": nullFunc_i, "nullFunc_vdddddd": nullFunc_vdddddd, "nullFunc_vdddd": nullFunc_vdddd, "nullFunc_vdd": nullFunc_vdd, "nullFunc_v": nullFunc_v, "nullFunc_viid": nullFunc_viid, "nullFunc_viiii": nullFunc_viiii, "invoke_viiiii": invoke_viiiii, "invoke_vd": invoke_vd, "invoke_vid": invoke_vid, "invoke_vi": invoke_vi, "invoke_vii": invoke_vii, "invoke_ii": invoke_ii, "invoke_viddd": invoke_viddd, "invoke_vidd": invoke_vidd, "invoke_iiii": invoke_iiii, "invoke_viiiiiiii": invoke_viiiiiiii, "invoke_viiiiii": invoke_viiiiii, "invoke_viii": invoke_viii, "invoke_vidddd": invoke_vidddd, "invoke_vdi": invoke_vdi, "invoke_viiiiiii": invoke_viiiiiii, "invoke_viiiiiiiii": invoke_viiiiiiiii, "invoke_iii": invoke_iii, "invoke_i": invoke_i, "invoke_vdddddd": invoke_vdddddd, "invoke_vdddd": invoke_vdddd, "invoke_vdd": invoke_vdd, "invoke_v": invoke_v, "invoke_viid": invoke_viid, "invoke_viiii": invoke_viiii, "_emscripten_glGetTexParameterfv": _emscripten_glGetTexParameterfv, "_glUseProgram": _glUseProgram, "_emscripten_glShaderSource": _emscripten_glShaderSource, "_glfwCreateWindow": _glfwCreateWindow, "_emscripten_glReleaseShaderCompiler": _emscripten_glReleaseShaderCompiler, "_emscripten_glBlendFuncSeparate": _emscripten_glBlendFuncSeparate, "_emscripten_glVertexAttribPointer": _emscripten_glVertexAttribPointer, "_emscripten_glGetIntegerv": _emscripten_glGetIntegerv, "_emscripten_glCullFace": _emscripten_glCullFace, "_emscripten_glIsProgram": _emscripten_glIsProgram, "_emscripten_glStencilMaskSeparate": _emscripten_glStencilMaskSeparate, "_emscripten_glViewport": _emscripten_glViewport, "_emscripten_glFrontFace": _emscripten_glFrontFace, "_glDeleteProgram": _glDeleteProgram, "_emscripten_glUniform3fv": _emscripten_glUniform3fv, "_emscripten_glPolygonOffset": _emscripten_glPolygonOffset, "_emscripten_glUseProgram": _emscripten_glUseProgram, "_emscripten_glBlendColor": _emscripten_glBlendColor, "_glBindBuffer": _glBindBuffer, "_emscripten_glDepthFunc": _emscripten_glDepthFunc, "_glGetShaderInfoLog": _glGetShaderInfoLog, "_emscripten_set_fullscreenchange_callback": _emscripten_set_fullscreenchange_callback, "_emscripten_set_touchmove_callback": _emscripten_set_touchmove_callback, "_emscripten_set_main_loop_timing": _emscripten_set_main_loop_timing, "_emscripten_set_gamepaddisconnected_callback": _emscripten_set_gamepaddisconnected_callback, "_glDisable": _glDisable, "_glBlendFunc": _glBlendFunc, "_emscripten_glDisableVertexAttribArray": _emscripten_glDisableVertexAttribArray, "_glGetAttribLocation": _glGetAttribLocation, "_glDisableVertexAttribArray": _glDisableVertexAttribArray, "_glCreateShader": _glCreateShader, "_emscripten_glSampleCoverage": _emscripten_glSampleCoverage, "_emscripten_glVertexPointer": _emscripten_glVertexPointer, "_emscripten_set_touchstart_callback": _emscripten_set_touchstart_callback, "emscriptenWebGLComputeImageSize": emscriptenWebGLComputeImageSize, "_emscripten_glGetBooleanv": _emscripten_glGetBooleanv, "_emscripten_glGetShaderSource": _emscripten_glGetShaderSource, "_glUniform4f": _glUniform4f, "_llvm_stacksave": _llvm_stacksave, "_emscripten_glUniform1i": _emscripten_glUniform1i, "_emscripten_glStencilFuncSeparate": _emscripten_glStencilFuncSeparate, "_emscripten_glFrustum": _emscripten_glFrustum, "_emscripten_glGenBuffers": _emscripten_glGenBuffers, "_emscripten_glDeleteObjectARB": _emscripten_glDeleteObjectARB, "_glfwSetWindowSizeCallback": _glfwSetWindowSizeCallback, "_emscripten_glGetShaderPrecisionFormat": _emscripten_glGetShaderPrecisionFormat, "_glfwInit": _glfwInit, "_glGenBuffers": _glGenBuffers, "_glShaderSource": _glShaderSource, "_emscripten_glGetString": _emscripten_glGetString, "_emscripten_glIsFramebuffer": _emscripten_glIsFramebuffer, "_emscripten_glIsEnabled": _emscripten_glIsEnabled, "_emscripten_glScissor": _emscripten_glScissor, "_emscripten_glVertexAttrib4fv": _emscripten_glVertexAttrib4fv, "_emscripten_glFramebufferTexture2D": _emscripten_glFramebufferTexture2D, "_emscripten_glTexParameteriv": _emscripten_glTexParameteriv, "_emscripten_glBindProgramARB": _emscripten_glBindProgramARB, "_emscripten_glStencilOpSeparate": _emscripten_glStencilOpSeparate, "_emscripten_glFramebufferRenderbuffer": _emscripten_glFramebufferRenderbuffer, "___syscall140": ___syscall140, "_glfwSetErrorCallback": _glfwSetErrorCallback, "_glfwDefaultWindowHints": _glfwDefaultWindowHints, "_glfwDestroyWindow": _glfwDestroyWindow, "_emscripten_glVertexAttrib1f": _emscripten_glVertexAttrib1f, "_emscripten_glGetActiveAttrib": _emscripten_glGetActiveAttrib, "_emscripten_glAttachShader": _emscripten_glAttachShader, "_glVertexAttribPointer": _glVertexAttribPointer, "_emscripten_glUniform2i": _emscripten_glUniform2i, "_emscripten_glUniform2f": _emscripten_glUniform2f, "_emscripten_glTexParameterfv": _emscripten_glTexParameterfv, "_emscripten_glUniformMatrix2fv": _emscripten_glUniformMatrix2fv, "_glGetProgramInfoLog": _glGetProgramInfoLog, "_glfwSetScrollCallback": _glfwSetScrollCallback, "_emscripten_glTexParameterf": _emscripten_glTexParameterf, "_emscripten_glGetAttachedShaders": _emscripten_glGetAttachedShaders, "_emscripten_glGenTextures": _emscripten_glGenTextures, "_emscripten_glTexParameteri": _emscripten_glTexParameteri, "_llvm_stackrestore": _llvm_stackrestore, "_glfwMakeContextCurrent": _glfwMakeContextCurrent, "_emscripten_glClear": _emscripten_glClear, "_glDrawElements": _glDrawElements, "_glBufferSubData": _glBufferSubData, "_emscripten_glValidateProgram": _emscripten_glValidateProgram, "_emscripten_glVertexAttrib2fv": _emscripten_glVertexAttrib2fv, "_glViewport": _glViewport, "_emscripten_glUniform4iv": _emscripten_glUniform4iv, "_emscripten_glGetTexParameteriv": _emscripten_glGetTexParameteriv, "___setErrNo": ___setErrNo, "_eglGetProcAddress": _eglGetProcAddress, "_emscripten_glBindAttribLocation": _emscripten_glBindAttribLocation, "_glDeleteTextures": _glDeleteTextures, "_glDepthFunc": _glDepthFunc, "_emscripten_glClientActiveTexture": _emscripten_glClientActiveTexture, "_emscripten_glVertexAttrib2f": _emscripten_glVertexAttrib2f, "_emscripten_glFlush": _emscripten_glFlush, "_emscripten_glCheckFramebufferStatus": _emscripten_glCheckFramebufferStatus, "_emscripten_glGenerateMipmap": _emscripten_glGenerateMipmap, "_emscripten_glGetError": _emscripten_glGetError, "_emscripten_glClearDepthf": _emscripten_glClearDepthf, "_emscripten_glBufferData": _emscripten_glBufferData, "_emscripten_glUniform3i": _emscripten_glUniform3i, "_emscripten_glRotatef": _emscripten_glRotatef, "_emscripten_glDeleteShader": _emscripten_glDeleteShader, "_glEnable": _glEnable, "_emscripten_glReadPixels": _emscripten_glReadPixels, "_emscripten_glMatrixMode": _emscripten_glMatrixMode, "_glGetString": _glGetString, "_emscripten_glClearStencil": _emscripten_glClearStencil, "_emscripten_glGetUniformLocation": _emscripten_glGetUniformLocation, "emscriptenWebGLGet": emscriptenWebGLGet, "_emscripten_glEnableVertexAttribArray": _emscripten_glEnableVertexAttribArray, "_emscripten_glGetAttribLocation": _emscripten_glGetAttribLocation, "_emscripten_get_now": _emscripten_get_now, "_emscripten_glNormalPointer": _emscripten_glNormalPointer, "_glAttachShader": _glAttachShader, "_emscripten_glTexCoordPointer": _emscripten_glTexCoordPointer, "_emscripten_glEnable": _emscripten_glEnable, "_glCreateProgram": _glCreateProgram, "_glUniformMatrix4fv": _glUniformMatrix4fv, "_emscripten_glClearDepth": _emscripten_glClearDepth, "___lock": ___lock, "emscriptenWebGLGetTexPixelData": emscriptenWebGLGetTexPixelData, "___syscall6": ___syscall6, "_emscripten_glIsBuffer": _emscripten_glIsBuffer, "_emscripten_glVertexAttrib3f": _emscripten_glVertexAttrib3f, "_time": _time, "___syscall146": ___syscall146, "_emscripten_glGetFramebufferAttachmentParameteriv": _emscripten_glGetFramebufferAttachmentParameteriv, "_emscripten_glBlendEquationSeparate": _emscripten_glBlendEquationSeparate, "_exit": _exit, "_emscripten_glEnableClientState": _emscripten_glEnableClientState, "_emscripten_glUniform4i": _emscripten_glUniform4i, "_emscripten_glDrawRangeElements": _emscripten_glDrawRangeElements, "_glCullFace": _glCullFace, "_emscripten_glGetPointerv": _emscripten_glGetPointerv, "_emscripten_set_keypress_callback": _emscripten_set_keypress_callback, "__emscripten_sample_gamepad_data": __emscripten_sample_gamepad_data, "_emscripten_get_gamepad_status": _emscripten_get_gamepad_status, "_emscripten_glUniform4f": _emscripten_glUniform4f, "_emscripten_glUniform2fv": _emscripten_glUniform2fv, "_glfwGetVideoModes": _glfwGetVideoModes, "_emscripten_set_click_callback": _emscripten_set_click_callback, "_emscripten_glShaderBinary": _emscripten_glShaderBinary, "_emscripten_glDrawElements": _emscripten_glDrawElements, "_emscripten_glBlendFunc": _emscripten_glBlendFunc, "_emscripten_get_num_gamepads": _emscripten_get_num_gamepads, "_glCompressedTexImage2D": _glCompressedTexImage2D, "_emscripten_glUniform1iv": _emscripten_glUniform1iv, "_emscripten_glGetVertexAttribPointerv": _emscripten_glGetVertexAttribPointerv, "_glClearDepthf": _glClearDepthf, "_emscripten_glCompressedTexSubImage2D": _emscripten_glCompressedTexSubImage2D, "emscriptenWebGLGetUniform": emscriptenWebGLGetUniform, "_emscripten_glGenRenderbuffers": _emscripten_glGenRenderbuffers, "_emscripten_glDeleteVertexArrays": _emscripten_glDeleteVertexArrays, "_glfwSetWindowShouldClose": _glfwSetWindowShouldClose, "_emscripten_glUniform1fv": _emscripten_glUniform1fv, "_emscripten_glGetActiveUniform": _emscripten_glGetActiveUniform, "_glBindTexture": _glBindTexture, "_emscripten_glUniform3iv": _emscripten_glUniform3iv, "_emscripten_glUniform2iv": _emscripten_glUniform2iv, "_emscripten_glHint": _emscripten_glHint, "_glfwSetCharCallback": _glfwSetCharCallback, "emscriptenWebGLGetVertexAttrib": emscriptenWebGLGetVertexAttrib, "_emscripten_glLoadMatrixf": _emscripten_glLoadMatrixf, "_emscripten_glDeleteProgram": _emscripten_glDeleteProgram, "_emscripten_glDeleteRenderbuffers": _emscripten_glDeleteRenderbuffers, "_emscripten_glDrawElementsInstanced": _emscripten_glDrawElementsInstanced, "_emscripten_glVertexAttrib4f": _emscripten_glVertexAttrib4f, "_glDrawArrays": _glDrawArrays, "_emscripten_glTexSubImage2D": _emscripten_glTexSubImage2D, "_emscripten_memcpy_big": _emscripten_memcpy_big, "_emscripten_glPixelStorei": _emscripten_glPixelStorei, "_glCompileShader": _glCompileShader, "_emscripten_get_pointerlock_status": _emscripten_get_pointerlock_status, "_emscripten_glUniformMatrix3fv": _emscripten_glUniformMatrix3fv, "_emscripten_glColorPointer": _emscripten_glColorPointer, "_emscripten_glGetBufferParameteriv": _emscripten_glGetBufferParameteriv, "_emscripten_glFinish": _emscripten_glFinish, "_emscripten_request_pointerlock": _emscripten_request_pointerlock, "_glGetFloatv": _glGetFloatv, "_emscripten_asm_const_iii": _emscripten_asm_const_iii, "_emscripten_glDepthMask": _emscripten_glDepthMask, "_glfwSetWindowIconifyCallback": _glfwSetWindowIconifyCallback, "_emscripten_glDrawBuffers": _emscripten_glDrawBuffers, "_glfwTerminate": _glfwTerminate, "_glFrontFace": _glFrontFace, "_emscripten_glGetObjectParameterivARB": _emscripten_glGetObjectParameterivARB, "_emscripten_exit_pointerlock": _emscripten_exit_pointerlock, "_glfwSwapInterval": _glfwSwapInterval, "_glUniform1i": _glUniform1i, "_glEnableVertexAttribArray": _glEnableVertexAttribArray, "_emscripten_glStencilFunc": _emscripten_glStencilFunc, "_abort": _abort, "_emscripten_glGetUniformiv": _emscripten_glGetUniformiv, "_glDeleteBuffers": _glDeleteBuffers, "_glBufferData": _glBufferData, "_glTexImage2D": _glTexImage2D, "_emscripten_glGetShaderiv": _emscripten_glGetShaderiv, "_glfwSetKeyCallback": _glfwSetKeyCallback, "_emscripten_glGenFramebuffers": _emscripten_glGenFramebuffers, "_emscripten_glUniformMatrix4fv": _emscripten_glUniformMatrix4fv, "_emscripten_glLoadIdentity": _emscripten_glLoadIdentity, "_glDeleteShader": _glDeleteShader, "_emscripten_glUniform1f": _emscripten_glUniform1f, "_glGetProgramiv": _glGetProgramiv, "_emscripten_glBindFramebuffer": _emscripten_glBindFramebuffer, "_emscripten_glIsRenderbuffer": _emscripten_glIsRenderbuffer, "_glfwGetTime": _glfwGetTime, "_emscripten_glRenderbufferStorage": _emscripten_glRenderbufferStorage, "_emscripten_set_gamepadconnected_callback": _emscripten_set_gamepadconnected_callback, "_emscripten_glGetVertexAttribiv": _emscripten_glGetVertexAttribiv, "_emscripten_glBindVertexArray": _emscripten_glBindVertexArray, "_emscripten_glDrawArraysInstanced": _emscripten_glDrawArraysInstanced, "_emscripten_set_touchcancel_callback": _emscripten_set_touchcancel_callback, "_emscripten_glCreateShader": _emscripten_glCreateShader, "_emscripten_glStencilMask": _emscripten_glStencilMask, "_emscripten_glDeleteTextures": _emscripten_glDeleteTextures, "_emscripten_glBindRenderbuffer": _emscripten_glBindRenderbuffer, "_glfwGetPrimaryMonitor": _glfwGetPrimaryMonitor, "_glLinkProgram": _glLinkProgram, "_emscripten_glVertexAttribDivisor": _emscripten_glVertexAttribDivisor, "_emscripten_set_touchend_callback": _emscripten_set_touchend_callback, "_emscripten_glGetUniformfv": _emscripten_glGetUniformfv, "_emscripten_glGetVertexAttribfv": _emscripten_glGetVertexAttribfv, "_emscripten_glGetRenderbufferParameteriv": _emscripten_glGetRenderbufferParameteriv, "_emscripten_glDeleteFramebuffers": _emscripten_glDeleteFramebuffers, "_glGetShaderiv": _glGetShaderiv, "_emscripten_glVertexAttrib3fv": _emscripten_glVertexAttrib3fv, "_glGetUniformLocation": _glGetUniformLocation, "_emscripten_glGetInfoLogARB": _emscripten_glGetInfoLogARB, "_emscripten_glCompileShader": _emscripten_glCompileShader, "_glClear": _glClear, "_glGenTextures": _glGenTextures, "_emscripten_glDisable": _emscripten_glDisable, "_emscripten_glDepthRangef": _emscripten_glDepthRangef, "__exit": __exit, "_emscripten_glLineWidth": _emscripten_glLineWidth, "_emscripten_glUniform3f": _emscripten_glUniform3f, "_emscripten_glGetShaderInfoLog": _emscripten_glGetShaderInfoLog, "_emscripten_glStencilOp": _emscripten_glStencilOp, "_glBindAttribLocation": _glBindAttribLocation, "_glPixelStorei": _glPixelStorei, "_emscripten_glColorMask": _emscripten_glColorMask, "_emscripten_glLinkProgram": _emscripten_glLinkProgram, "_emscripten_glBlendEquation": _emscripten_glBlendEquation, "_emscripten_glIsTexture": _emscripten_glIsTexture, "_emscripten_glGetProgramiv": _emscripten_glGetProgramiv, "_emscripten_glVertexAttrib1fv": _emscripten_glVertexAttrib1fv, "_emscripten_glBindTexture": _emscripten_glBindTexture, "_glfwSetMouseButtonCallback": _glfwSetMouseButtonCallback, "_glfwGetCursorPos": _glfwGetCursorPos, "_emscripten_glActiveTexture": _emscripten_glActiveTexture, "_emscripten_glDeleteBuffers": _emscripten_glDeleteBuffers, "___syscall54": ___syscall54, "___unlock": ___unlock, "_emscripten_glBufferSubData": _emscripten_glBufferSubData, "_glfwSwapBuffers": _glfwSwapBuffers, "_emscripten_glDepthRange": _emscripten_glDepthRange, "_emscripten_set_main_loop": _emscripten_set_main_loop, "_emscripten_glGetProgramInfoLog": _emscripten_glGetProgramInfoLog, "_glfwWindowHint": _glfwWindowHint, "_emscripten_glIsShader": _emscripten_glIsShader, "_emscripten_glUniform4fv": _emscripten_glUniform4fv, "_emscripten_glGenVertexArrays": _emscripten_glGenVertexArrays, "_emscripten_glDrawArrays": _emscripten_glDrawArrays, "_emscripten_glCompressedTexImage2D": _emscripten_glCompressedTexImage2D, "_emscripten_glClearColor": _emscripten_glClearColor, "_emscripten_glCreateProgram": _emscripten_glCreateProgram, "_emscripten_glCopyTexSubImage2D": _emscripten_glCopyTexSubImage2D, "_glTexParameteri": _glTexParameteri, "_emscripten_glBindBuffer": _emscripten_glBindBuffer, "_emscripten_glGetFloatv": _emscripten_glGetFloatv, "_emscripten_glDetachShader": _emscripten_glDetachShader, "_glClearColor": _glClearColor, "_glfwSetCursorPosCallback": _glfwSetCursorPosCallback, "_glfwSetCursorEnterCallback": _glfwSetCursorEnterCallback, "_emscripten_glCopyTexImage2D": _emscripten_glCopyTexImage2D, "_emscripten_glTexImage2D": _emscripten_glTexImage2D, "DYNAMICTOP_PTR": DYNAMICTOP_PTR, "tempDoublePtr": tempDoublePtr, "ABORT": ABORT, "STACKTOP": STACKTOP, "STACK_MAX": STACK_MAX, "cttz_i8": cttz_i8 };
// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
  'use asm';
  
  
  var HEAP8 = new global.Int8Array(buffer);
  var HEAP16 = new global.Int16Array(buffer);
  var HEAP32 = new global.Int32Array(buffer);
  var HEAPU8 = new global.Uint8Array(buffer);
  var HEAPU16 = new global.Uint16Array(buffer);
  var HEAPU32 = new global.Uint32Array(buffer);
  var HEAPF32 = new global.Float32Array(buffer);
  var HEAPF64 = new global.Float64Array(buffer);


  var DYNAMICTOP_PTR=env.DYNAMICTOP_PTR|0;
  var tempDoublePtr=env.tempDoublePtr|0;
  var ABORT=env.ABORT|0;
  var STACKTOP=env.STACKTOP|0;
  var STACK_MAX=env.STACK_MAX|0;
  var cttz_i8=env.cttz_i8|0;

  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var nan = global.NaN, inf = global.Infinity;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
  var tempRet0 = 0;

  var Math_floor=global.Math.floor;
  var Math_abs=global.Math.abs;
  var Math_sqrt=global.Math.sqrt;
  var Math_pow=global.Math.pow;
  var Math_cos=global.Math.cos;
  var Math_sin=global.Math.sin;
  var Math_tan=global.Math.tan;
  var Math_acos=global.Math.acos;
  var Math_asin=global.Math.asin;
  var Math_atan=global.Math.atan;
  var Math_atan2=global.Math.atan2;
  var Math_exp=global.Math.exp;
  var Math_log=global.Math.log;
  var Math_ceil=global.Math.ceil;
  var Math_imul=global.Math.imul;
  var Math_min=global.Math.min;
  var Math_max=global.Math.max;
  var Math_clz32=global.Math.clz32;
  var abort=env.abort;
  var assert=env.assert;
  var enlargeMemory=env.enlargeMemory;
  var getTotalMemory=env.getTotalMemory;
  var abortOnCannotGrowMemory=env.abortOnCannotGrowMemory;
  var abortStackOverflow=env.abortStackOverflow;
  var nullFunc_viiiii=env.nullFunc_viiiii;
  var nullFunc_vd=env.nullFunc_vd;
  var nullFunc_vid=env.nullFunc_vid;
  var nullFunc_vi=env.nullFunc_vi;
  var nullFunc_vii=env.nullFunc_vii;
  var nullFunc_ii=env.nullFunc_ii;
  var nullFunc_viddd=env.nullFunc_viddd;
  var nullFunc_vidd=env.nullFunc_vidd;
  var nullFunc_iiii=env.nullFunc_iiii;
  var nullFunc_viiiiiiii=env.nullFunc_viiiiiiii;
  var nullFunc_viiiiii=env.nullFunc_viiiiii;
  var nullFunc_viii=env.nullFunc_viii;
  var nullFunc_vidddd=env.nullFunc_vidddd;
  var nullFunc_vdi=env.nullFunc_vdi;
  var nullFunc_viiiiiii=env.nullFunc_viiiiiii;
  var nullFunc_viiiiiiiii=env.nullFunc_viiiiiiiii;
  var nullFunc_iii=env.nullFunc_iii;
  var nullFunc_i=env.nullFunc_i;
  var nullFunc_vdddddd=env.nullFunc_vdddddd;
  var nullFunc_vdddd=env.nullFunc_vdddd;
  var nullFunc_vdd=env.nullFunc_vdd;
  var nullFunc_v=env.nullFunc_v;
  var nullFunc_viid=env.nullFunc_viid;
  var nullFunc_viiii=env.nullFunc_viiii;
  var invoke_viiiii=env.invoke_viiiii;
  var invoke_vd=env.invoke_vd;
  var invoke_vid=env.invoke_vid;
  var invoke_vi=env.invoke_vi;
  var invoke_vii=env.invoke_vii;
  var invoke_ii=env.invoke_ii;
  var invoke_viddd=env.invoke_viddd;
  var invoke_vidd=env.invoke_vidd;
  var invoke_iiii=env.invoke_iiii;
  var invoke_viiiiiiii=env.invoke_viiiiiiii;
  var invoke_viiiiii=env.invoke_viiiiii;
  var invoke_viii=env.invoke_viii;
  var invoke_vidddd=env.invoke_vidddd;
  var invoke_vdi=env.invoke_vdi;
  var invoke_viiiiiii=env.invoke_viiiiiii;
  var invoke_viiiiiiiii=env.invoke_viiiiiiiii;
  var invoke_iii=env.invoke_iii;
  var invoke_i=env.invoke_i;
  var invoke_vdddddd=env.invoke_vdddddd;
  var invoke_vdddd=env.invoke_vdddd;
  var invoke_vdd=env.invoke_vdd;
  var invoke_v=env.invoke_v;
  var invoke_viid=env.invoke_viid;
  var invoke_viiii=env.invoke_viiii;
  var _emscripten_glGetTexParameterfv=env._emscripten_glGetTexParameterfv;
  var _glUseProgram=env._glUseProgram;
  var _emscripten_glShaderSource=env._emscripten_glShaderSource;
  var _glfwCreateWindow=env._glfwCreateWindow;
  var _emscripten_glReleaseShaderCompiler=env._emscripten_glReleaseShaderCompiler;
  var _emscripten_glBlendFuncSeparate=env._emscripten_glBlendFuncSeparate;
  var _emscripten_glVertexAttribPointer=env._emscripten_glVertexAttribPointer;
  var _emscripten_glGetIntegerv=env._emscripten_glGetIntegerv;
  var _emscripten_glCullFace=env._emscripten_glCullFace;
  var _emscripten_glIsProgram=env._emscripten_glIsProgram;
  var _emscripten_glStencilMaskSeparate=env._emscripten_glStencilMaskSeparate;
  var _emscripten_glViewport=env._emscripten_glViewport;
  var _emscripten_glFrontFace=env._emscripten_glFrontFace;
  var _glDeleteProgram=env._glDeleteProgram;
  var _emscripten_glUniform3fv=env._emscripten_glUniform3fv;
  var _emscripten_glPolygonOffset=env._emscripten_glPolygonOffset;
  var _emscripten_glUseProgram=env._emscripten_glUseProgram;
  var _emscripten_glBlendColor=env._emscripten_glBlendColor;
  var _glBindBuffer=env._glBindBuffer;
  var _emscripten_glDepthFunc=env._emscripten_glDepthFunc;
  var _glGetShaderInfoLog=env._glGetShaderInfoLog;
  var _emscripten_set_fullscreenchange_callback=env._emscripten_set_fullscreenchange_callback;
  var _emscripten_set_touchmove_callback=env._emscripten_set_touchmove_callback;
  var _emscripten_set_main_loop_timing=env._emscripten_set_main_loop_timing;
  var _emscripten_set_gamepaddisconnected_callback=env._emscripten_set_gamepaddisconnected_callback;
  var _glDisable=env._glDisable;
  var _glBlendFunc=env._glBlendFunc;
  var _emscripten_glDisableVertexAttribArray=env._emscripten_glDisableVertexAttribArray;
  var _glGetAttribLocation=env._glGetAttribLocation;
  var _glDisableVertexAttribArray=env._glDisableVertexAttribArray;
  var _glCreateShader=env._glCreateShader;
  var _emscripten_glSampleCoverage=env._emscripten_glSampleCoverage;
  var _emscripten_glVertexPointer=env._emscripten_glVertexPointer;
  var _emscripten_set_touchstart_callback=env._emscripten_set_touchstart_callback;
  var emscriptenWebGLComputeImageSize=env.emscriptenWebGLComputeImageSize;
  var _emscripten_glGetBooleanv=env._emscripten_glGetBooleanv;
  var _emscripten_glGetShaderSource=env._emscripten_glGetShaderSource;
  var _glUniform4f=env._glUniform4f;
  var _llvm_stacksave=env._llvm_stacksave;
  var _emscripten_glUniform1i=env._emscripten_glUniform1i;
  var _emscripten_glStencilFuncSeparate=env._emscripten_glStencilFuncSeparate;
  var _emscripten_glFrustum=env._emscripten_glFrustum;
  var _emscripten_glGenBuffers=env._emscripten_glGenBuffers;
  var _emscripten_glDeleteObjectARB=env._emscripten_glDeleteObjectARB;
  var _glfwSetWindowSizeCallback=env._glfwSetWindowSizeCallback;
  var _emscripten_glGetShaderPrecisionFormat=env._emscripten_glGetShaderPrecisionFormat;
  var _glfwInit=env._glfwInit;
  var _glGenBuffers=env._glGenBuffers;
  var _glShaderSource=env._glShaderSource;
  var _emscripten_glGetString=env._emscripten_glGetString;
  var _emscripten_glIsFramebuffer=env._emscripten_glIsFramebuffer;
  var _emscripten_glIsEnabled=env._emscripten_glIsEnabled;
  var _emscripten_glScissor=env._emscripten_glScissor;
  var _emscripten_glVertexAttrib4fv=env._emscripten_glVertexAttrib4fv;
  var _emscripten_glFramebufferTexture2D=env._emscripten_glFramebufferTexture2D;
  var _emscripten_glTexParameteriv=env._emscripten_glTexParameteriv;
  var _emscripten_glBindProgramARB=env._emscripten_glBindProgramARB;
  var _emscripten_glStencilOpSeparate=env._emscripten_glStencilOpSeparate;
  var _emscripten_glFramebufferRenderbuffer=env._emscripten_glFramebufferRenderbuffer;
  var ___syscall140=env.___syscall140;
  var _glfwSetErrorCallback=env._glfwSetErrorCallback;
  var _glfwDefaultWindowHints=env._glfwDefaultWindowHints;
  var _glfwDestroyWindow=env._glfwDestroyWindow;
  var _emscripten_glVertexAttrib1f=env._emscripten_glVertexAttrib1f;
  var _emscripten_glGetActiveAttrib=env._emscripten_glGetActiveAttrib;
  var _emscripten_glAttachShader=env._emscripten_glAttachShader;
  var _glVertexAttribPointer=env._glVertexAttribPointer;
  var _emscripten_glUniform2i=env._emscripten_glUniform2i;
  var _emscripten_glUniform2f=env._emscripten_glUniform2f;
  var _emscripten_glTexParameterfv=env._emscripten_glTexParameterfv;
  var _emscripten_glUniformMatrix2fv=env._emscripten_glUniformMatrix2fv;
  var _glGetProgramInfoLog=env._glGetProgramInfoLog;
  var _glfwSetScrollCallback=env._glfwSetScrollCallback;
  var _emscripten_glTexParameterf=env._emscripten_glTexParameterf;
  var _emscripten_glGetAttachedShaders=env._emscripten_glGetAttachedShaders;
  var _emscripten_glGenTextures=env._emscripten_glGenTextures;
  var _emscripten_glTexParameteri=env._emscripten_glTexParameteri;
  var _llvm_stackrestore=env._llvm_stackrestore;
  var _glfwMakeContextCurrent=env._glfwMakeContextCurrent;
  var _emscripten_glClear=env._emscripten_glClear;
  var _glDrawElements=env._glDrawElements;
  var _glBufferSubData=env._glBufferSubData;
  var _emscripten_glValidateProgram=env._emscripten_glValidateProgram;
  var _emscripten_glVertexAttrib2fv=env._emscripten_glVertexAttrib2fv;
  var _glViewport=env._glViewport;
  var _emscripten_glUniform4iv=env._emscripten_glUniform4iv;
  var _emscripten_glGetTexParameteriv=env._emscripten_glGetTexParameteriv;
  var ___setErrNo=env.___setErrNo;
  var _eglGetProcAddress=env._eglGetProcAddress;
  var _emscripten_glBindAttribLocation=env._emscripten_glBindAttribLocation;
  var _glDeleteTextures=env._glDeleteTextures;
  var _glDepthFunc=env._glDepthFunc;
  var _emscripten_glClientActiveTexture=env._emscripten_glClientActiveTexture;
  var _emscripten_glVertexAttrib2f=env._emscripten_glVertexAttrib2f;
  var _emscripten_glFlush=env._emscripten_glFlush;
  var _emscripten_glCheckFramebufferStatus=env._emscripten_glCheckFramebufferStatus;
  var _emscripten_glGenerateMipmap=env._emscripten_glGenerateMipmap;
  var _emscripten_glGetError=env._emscripten_glGetError;
  var _emscripten_glClearDepthf=env._emscripten_glClearDepthf;
  var _emscripten_glBufferData=env._emscripten_glBufferData;
  var _emscripten_glUniform3i=env._emscripten_glUniform3i;
  var _emscripten_glRotatef=env._emscripten_glRotatef;
  var _emscripten_glDeleteShader=env._emscripten_glDeleteShader;
  var _glEnable=env._glEnable;
  var _emscripten_glReadPixels=env._emscripten_glReadPixels;
  var _emscripten_glMatrixMode=env._emscripten_glMatrixMode;
  var _glGetString=env._glGetString;
  var _emscripten_glClearStencil=env._emscripten_glClearStencil;
  var _emscripten_glGetUniformLocation=env._emscripten_glGetUniformLocation;
  var emscriptenWebGLGet=env.emscriptenWebGLGet;
  var _emscripten_glEnableVertexAttribArray=env._emscripten_glEnableVertexAttribArray;
  var _emscripten_glGetAttribLocation=env._emscripten_glGetAttribLocation;
  var _emscripten_get_now=env._emscripten_get_now;
  var _emscripten_glNormalPointer=env._emscripten_glNormalPointer;
  var _glAttachShader=env._glAttachShader;
  var _emscripten_glTexCoordPointer=env._emscripten_glTexCoordPointer;
  var _emscripten_glEnable=env._emscripten_glEnable;
  var _glCreateProgram=env._glCreateProgram;
  var _glUniformMatrix4fv=env._glUniformMatrix4fv;
  var _emscripten_glClearDepth=env._emscripten_glClearDepth;
  var ___lock=env.___lock;
  var emscriptenWebGLGetTexPixelData=env.emscriptenWebGLGetTexPixelData;
  var ___syscall6=env.___syscall6;
  var _emscripten_glIsBuffer=env._emscripten_glIsBuffer;
  var _emscripten_glVertexAttrib3f=env._emscripten_glVertexAttrib3f;
  var _time=env._time;
  var ___syscall146=env.___syscall146;
  var _emscripten_glGetFramebufferAttachmentParameteriv=env._emscripten_glGetFramebufferAttachmentParameteriv;
  var _emscripten_glBlendEquationSeparate=env._emscripten_glBlendEquationSeparate;
  var _exit=env._exit;
  var _emscripten_glEnableClientState=env._emscripten_glEnableClientState;
  var _emscripten_glUniform4i=env._emscripten_glUniform4i;
  var _emscripten_glDrawRangeElements=env._emscripten_glDrawRangeElements;
  var _glCullFace=env._glCullFace;
  var _emscripten_glGetPointerv=env._emscripten_glGetPointerv;
  var _emscripten_set_keypress_callback=env._emscripten_set_keypress_callback;
  var __emscripten_sample_gamepad_data=env.__emscripten_sample_gamepad_data;
  var _emscripten_get_gamepad_status=env._emscripten_get_gamepad_status;
  var _emscripten_glUniform4f=env._emscripten_glUniform4f;
  var _emscripten_glUniform2fv=env._emscripten_glUniform2fv;
  var _glfwGetVideoModes=env._glfwGetVideoModes;
  var _emscripten_set_click_callback=env._emscripten_set_click_callback;
  var _emscripten_glShaderBinary=env._emscripten_glShaderBinary;
  var _emscripten_glDrawElements=env._emscripten_glDrawElements;
  var _emscripten_glBlendFunc=env._emscripten_glBlendFunc;
  var _emscripten_get_num_gamepads=env._emscripten_get_num_gamepads;
  var _glCompressedTexImage2D=env._glCompressedTexImage2D;
  var _emscripten_glUniform1iv=env._emscripten_glUniform1iv;
  var _emscripten_glGetVertexAttribPointerv=env._emscripten_glGetVertexAttribPointerv;
  var _glClearDepthf=env._glClearDepthf;
  var _emscripten_glCompressedTexSubImage2D=env._emscripten_glCompressedTexSubImage2D;
  var emscriptenWebGLGetUniform=env.emscriptenWebGLGetUniform;
  var _emscripten_glGenRenderbuffers=env._emscripten_glGenRenderbuffers;
  var _emscripten_glDeleteVertexArrays=env._emscripten_glDeleteVertexArrays;
  var _glfwSetWindowShouldClose=env._glfwSetWindowShouldClose;
  var _emscripten_glUniform1fv=env._emscripten_glUniform1fv;
  var _emscripten_glGetActiveUniform=env._emscripten_glGetActiveUniform;
  var _glBindTexture=env._glBindTexture;
  var _emscripten_glUniform3iv=env._emscripten_glUniform3iv;
  var _emscripten_glUniform2iv=env._emscripten_glUniform2iv;
  var _emscripten_glHint=env._emscripten_glHint;
  var _glfwSetCharCallback=env._glfwSetCharCallback;
  var emscriptenWebGLGetVertexAttrib=env.emscriptenWebGLGetVertexAttrib;
  var _emscripten_glLoadMatrixf=env._emscripten_glLoadMatrixf;
  var _emscripten_glDeleteProgram=env._emscripten_glDeleteProgram;
  var _emscripten_glDeleteRenderbuffers=env._emscripten_glDeleteRenderbuffers;
  var _emscripten_glDrawElementsInstanced=env._emscripten_glDrawElementsInstanced;
  var _emscripten_glVertexAttrib4f=env._emscripten_glVertexAttrib4f;
  var _glDrawArrays=env._glDrawArrays;
  var _emscripten_glTexSubImage2D=env._emscripten_glTexSubImage2D;
  var _emscripten_memcpy_big=env._emscripten_memcpy_big;
  var _emscripten_glPixelStorei=env._emscripten_glPixelStorei;
  var _glCompileShader=env._glCompileShader;
  var _emscripten_get_pointerlock_status=env._emscripten_get_pointerlock_status;
  var _emscripten_glUniformMatrix3fv=env._emscripten_glUniformMatrix3fv;
  var _emscripten_glColorPointer=env._emscripten_glColorPointer;
  var _emscripten_glGetBufferParameteriv=env._emscripten_glGetBufferParameteriv;
  var _emscripten_glFinish=env._emscripten_glFinish;
  var _emscripten_request_pointerlock=env._emscripten_request_pointerlock;
  var _glGetFloatv=env._glGetFloatv;
  var _emscripten_asm_const_iii=env._emscripten_asm_const_iii;
  var _emscripten_glDepthMask=env._emscripten_glDepthMask;
  var _glfwSetWindowIconifyCallback=env._glfwSetWindowIconifyCallback;
  var _emscripten_glDrawBuffers=env._emscripten_glDrawBuffers;
  var _glfwTerminate=env._glfwTerminate;
  var _glFrontFace=env._glFrontFace;
  var _emscripten_glGetObjectParameterivARB=env._emscripten_glGetObjectParameterivARB;
  var _emscripten_exit_pointerlock=env._emscripten_exit_pointerlock;
  var _glfwSwapInterval=env._glfwSwapInterval;
  var _glUniform1i=env._glUniform1i;
  var _glEnableVertexAttribArray=env._glEnableVertexAttribArray;
  var _emscripten_glStencilFunc=env._emscripten_glStencilFunc;
  var _abort=env._abort;
  var _emscripten_glGetUniformiv=env._emscripten_glGetUniformiv;
  var _glDeleteBuffers=env._glDeleteBuffers;
  var _glBufferData=env._glBufferData;
  var _glTexImage2D=env._glTexImage2D;
  var _emscripten_glGetShaderiv=env._emscripten_glGetShaderiv;
  var _glfwSetKeyCallback=env._glfwSetKeyCallback;
  var _emscripten_glGenFramebuffers=env._emscripten_glGenFramebuffers;
  var _emscripten_glUniformMatrix4fv=env._emscripten_glUniformMatrix4fv;
  var _emscripten_glLoadIdentity=env._emscripten_glLoadIdentity;
  var _glDeleteShader=env._glDeleteShader;
  var _emscripten_glUniform1f=env._emscripten_glUniform1f;
  var _glGetProgramiv=env._glGetProgramiv;
  var _emscripten_glBindFramebuffer=env._emscripten_glBindFramebuffer;
  var _emscripten_glIsRenderbuffer=env._emscripten_glIsRenderbuffer;
  var _glfwGetTime=env._glfwGetTime;
  var _emscripten_glRenderbufferStorage=env._emscripten_glRenderbufferStorage;
  var _emscripten_set_gamepadconnected_callback=env._emscripten_set_gamepadconnected_callback;
  var _emscripten_glGetVertexAttribiv=env._emscripten_glGetVertexAttribiv;
  var _emscripten_glBindVertexArray=env._emscripten_glBindVertexArray;
  var _emscripten_glDrawArraysInstanced=env._emscripten_glDrawArraysInstanced;
  var _emscripten_set_touchcancel_callback=env._emscripten_set_touchcancel_callback;
  var _emscripten_glCreateShader=env._emscripten_glCreateShader;
  var _emscripten_glStencilMask=env._emscripten_glStencilMask;
  var _emscripten_glDeleteTextures=env._emscripten_glDeleteTextures;
  var _emscripten_glBindRenderbuffer=env._emscripten_glBindRenderbuffer;
  var _glfwGetPrimaryMonitor=env._glfwGetPrimaryMonitor;
  var _glLinkProgram=env._glLinkProgram;
  var _emscripten_glVertexAttribDivisor=env._emscripten_glVertexAttribDivisor;
  var _emscripten_set_touchend_callback=env._emscripten_set_touchend_callback;
  var _emscripten_glGetUniformfv=env._emscripten_glGetUniformfv;
  var _emscripten_glGetVertexAttribfv=env._emscripten_glGetVertexAttribfv;
  var _emscripten_glGetRenderbufferParameteriv=env._emscripten_glGetRenderbufferParameteriv;
  var _emscripten_glDeleteFramebuffers=env._emscripten_glDeleteFramebuffers;
  var _glGetShaderiv=env._glGetShaderiv;
  var _emscripten_glVertexAttrib3fv=env._emscripten_glVertexAttrib3fv;
  var _glGetUniformLocation=env._glGetUniformLocation;
  var _emscripten_glGetInfoLogARB=env._emscripten_glGetInfoLogARB;
  var _emscripten_glCompileShader=env._emscripten_glCompileShader;
  var _glClear=env._glClear;
  var _glGenTextures=env._glGenTextures;
  var _emscripten_glDisable=env._emscripten_glDisable;
  var _emscripten_glDepthRangef=env._emscripten_glDepthRangef;
  var __exit=env.__exit;
  var _emscripten_glLineWidth=env._emscripten_glLineWidth;
  var _emscripten_glUniform3f=env._emscripten_glUniform3f;
  var _emscripten_glGetShaderInfoLog=env._emscripten_glGetShaderInfoLog;
  var _emscripten_glStencilOp=env._emscripten_glStencilOp;
  var _glBindAttribLocation=env._glBindAttribLocation;
  var _glPixelStorei=env._glPixelStorei;
  var _emscripten_glColorMask=env._emscripten_glColorMask;
  var _emscripten_glLinkProgram=env._emscripten_glLinkProgram;
  var _emscripten_glBlendEquation=env._emscripten_glBlendEquation;
  var _emscripten_glIsTexture=env._emscripten_glIsTexture;
  var _emscripten_glGetProgramiv=env._emscripten_glGetProgramiv;
  var _emscripten_glVertexAttrib1fv=env._emscripten_glVertexAttrib1fv;
  var _emscripten_glBindTexture=env._emscripten_glBindTexture;
  var _glfwSetMouseButtonCallback=env._glfwSetMouseButtonCallback;
  var _glfwGetCursorPos=env._glfwGetCursorPos;
  var _emscripten_glActiveTexture=env._emscripten_glActiveTexture;
  var _emscripten_glDeleteBuffers=env._emscripten_glDeleteBuffers;
  var ___syscall54=env.___syscall54;
  var ___unlock=env.___unlock;
  var _emscripten_glBufferSubData=env._emscripten_glBufferSubData;
  var _glfwSwapBuffers=env._glfwSwapBuffers;
  var _emscripten_glDepthRange=env._emscripten_glDepthRange;
  var _emscripten_set_main_loop=env._emscripten_set_main_loop;
  var _emscripten_glGetProgramInfoLog=env._emscripten_glGetProgramInfoLog;
  var _glfwWindowHint=env._glfwWindowHint;
  var _emscripten_glIsShader=env._emscripten_glIsShader;
  var _emscripten_glUniform4fv=env._emscripten_glUniform4fv;
  var _emscripten_glGenVertexArrays=env._emscripten_glGenVertexArrays;
  var _emscripten_glDrawArrays=env._emscripten_glDrawArrays;
  var _emscripten_glCompressedTexImage2D=env._emscripten_glCompressedTexImage2D;
  var _emscripten_glClearColor=env._emscripten_glClearColor;
  var _emscripten_glCreateProgram=env._emscripten_glCreateProgram;
  var _emscripten_glCopyTexSubImage2D=env._emscripten_glCopyTexSubImage2D;
  var _glTexParameteri=env._glTexParameteri;
  var _emscripten_glBindBuffer=env._emscripten_glBindBuffer;
  var _emscripten_glGetFloatv=env._emscripten_glGetFloatv;
  var _emscripten_glDetachShader=env._emscripten_glDetachShader;
  var _glClearColor=env._glClearColor;
  var _glfwSetCursorPosCallback=env._glfwSetCursorPosCallback;
  var _glfwSetCursorEnterCallback=env._glfwSetCursorEnterCallback;
  var _emscripten_glCopyTexImage2D=env._emscripten_glCopyTexImage2D;
  var _emscripten_glTexImage2D=env._emscripten_glTexImage2D;
  var tempFloat = 0.0;

// EMSCRIPTEN_START_FUNCS

function stackAlloc(size) {
  size = size|0;
  var ret = 0;
  ret = STACKTOP;
  STACKTOP = (STACKTOP + size)|0;
  STACKTOP = (STACKTOP + 15)&-16;
  if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(size|0);

  return ret|0;
}
function stackSave() {
  return STACKTOP|0;
}
function stackRestore(top) {
  top = top|0;
  STACKTOP = top;
}
function establishStackSpace(stackBase, stackMax) {
  stackBase = stackBase|0;
  stackMax = stackMax|0;
  STACKTOP = stackBase;
  STACK_MAX = stackMax;
}

function setThrew(threw, value) {
  threw = threw|0;
  value = value|0;
  if ((__THREW__|0) == 0) {
    __THREW__ = threw;
    threwValue = value;
  }
}

function setTempRet0(value) {
  value = value|0;
  tempRet0 = value;
}
function getTempRet0() {
  return tempRet0|0;
}

function _main() {
 var $0 = 0, $1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[2]|0;
 $1 = HEAP32[3]|0;
 _InitWindow($0,$1,3492);
 _emscripten_set_main_loop((1|0),0,1);
 _CloseWindow();
 return 0;
}
function _UpdateDrawFrame() {
 var $$byval_copy13 = 0, $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0.0, $24 = 0.0, $25 = 0.0;
 var $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer4 = 0, $vararg_buffer7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $$byval_copy13 = sp + 52|0;
 $vararg_buffer7 = sp + 24|0;
 $vararg_buffer4 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $0 = sp + 48|0;
 $1 = sp + 44|0;
 $2 = sp + 40|0;
 $3 = sp + 36|0;
 $4 = sp + 32|0;
 _BeginDrawing();
 HEAP8[$0>>0] = -11;
 $5 = ((($0)) + 1|0);
 HEAP8[$5>>0] = -11;
 $6 = ((($0)) + 2|0);
 HEAP8[$6>>0] = -11;
 $7 = ((($0)) + 3|0);
 HEAP8[$7>>0] = -1;
 ;HEAP8[$$byval_copy13>>0]=HEAP8[$0>>0]|0;HEAP8[$$byval_copy13+1>>0]=HEAP8[$0+1>>0]|0;HEAP8[$$byval_copy13+2>>0]=HEAP8[$0+2>>0]|0;HEAP8[$$byval_copy13+3>>0]=HEAP8[$0+3>>0]|0;
 _ClearBackground($$byval_copy13);
 $8 = HEAP32[4]|0;
 HEAP32[$vararg_buffer>>2] = $8;
 $9 = (_FormatText(3532,$vararg_buffer)|0);
 HEAP8[$1>>0] = -26;
 $10 = ((($1)) + 1|0);
 HEAP8[$10>>0] = 41;
 $11 = ((($1)) + 2|0);
 HEAP8[$11>>0] = 55;
 $12 = ((($1)) + 3|0);
 HEAP8[$12>>0] = -1;
 ;HEAP8[$$byval_copy13>>0]=HEAP8[$1>>0]|0;HEAP8[$$byval_copy13+1>>0]=HEAP8[$1+1>>0]|0;HEAP8[$$byval_copy13+2>>0]=HEAP8[$1+2>>0]|0;HEAP8[$$byval_copy13+3>>0]=HEAP8[$1+3>>0]|0;
 _DrawText($9,200,80,20,$$byval_copy13);
 $13 = HEAP32[5]|0;
 HEAP32[$vararg_buffer1>>2] = $13;
 $14 = (_FormatText(3544,$vararg_buffer1)|0);
 HEAP8[$2>>0] = 0;
 $15 = ((($2)) + 1|0);
 HEAP8[$15>>0] = -28;
 $16 = ((($2)) + 2|0);
 HEAP8[$16>>0] = 48;
 $17 = ((($2)) + 3|0);
 HEAP8[$17>>0] = -1;
 ;HEAP8[$$byval_copy13>>0]=HEAP8[$2>>0]|0;HEAP8[$$byval_copy13+1>>0]=HEAP8[$2+1>>0]|0;HEAP8[$$byval_copy13+2>>0]=HEAP8[$2+2>>0]|0;HEAP8[$$byval_copy13+3>>0]=HEAP8[$2+3>>0]|0;
 _DrawText($14,200,120,20,$$byval_copy13);
 $18 = HEAP32[6]|0;
 HEAP32[$vararg_buffer4>>2] = $18;
 $19 = (_FormatText(3558,$vararg_buffer4)|0);
 HEAP8[$3>>0] = 0;
 $20 = ((($3)) + 1|0);
 HEAP8[$20>>0] = 121;
 $21 = ((($3)) + 2|0);
 HEAP8[$21>>0] = -15;
 $22 = ((($3)) + 3|0);
 HEAP8[$22>>0] = -1;
 ;HEAP8[$$byval_copy13>>0]=HEAP8[$3>>0]|0;HEAP8[$$byval_copy13+1>>0]=HEAP8[$3+1>>0]|0;HEAP8[$$byval_copy13+2>>0]=HEAP8[$3+2>>0]|0;HEAP8[$$byval_copy13+3>>0]=HEAP8[$3+3>>0]|0;
 _DrawText($19,200,160,40,$$byval_copy13);
 $23 = (+_GetFrameTime());
 $24 = $23 * 1000.0;
 $25 = $24;
 HEAPF64[$vararg_buffer7>>3] = $25;
 $26 = (_FormatText(3570,$vararg_buffer7)|0);
 HEAP8[$4>>0] = 0;
 $27 = ((($4)) + 1|0);
 HEAP8[$27>>0] = 0;
 $28 = ((($4)) + 2|0);
 HEAP8[$28>>0] = 0;
 $29 = ((($4)) + 3|0);
 HEAP8[$29>>0] = -1;
 ;HEAP8[$$byval_copy13>>0]=HEAP8[$4>>0]|0;HEAP8[$$byval_copy13+1>>0]=HEAP8[$4+1>>0]|0;HEAP8[$$byval_copy13+2>>0]=HEAP8[$4+2>>0]|0;HEAP8[$$byval_copy13+3>>0]=HEAP8[$4+3>>0]|0;
 _DrawText($26,200,220,20,$$byval_copy13);
 _EndDrawing();
 STACKTOP = sp;return;
}
function _Vector2Distance($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $10 = 0.0, $11 = 0.0, $12 = 0.0, $13 = 0.0, $2 = 0.0, $3 = 0.0, $4 = 0.0, $5 = 0.0, $6 = 0, $7 = 0.0, $8 = 0, $9 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = +HEAPF32[$0>>2];
 $3 = +HEAPF32[$1>>2];
 $4 = $2 - $3;
 $5 = $4 * $4;
 $6 = ((($0)) + 4|0);
 $7 = +HEAPF32[$6>>2];
 $8 = ((($1)) + 4|0);
 $9 = +HEAPF32[$8>>2];
 $10 = $7 - $9;
 $11 = $10 * $10;
 $12 = $5 + $11;
 $13 = (+Math_sqrt((+$12)));
 return (+$13);
}
function _Vector2Angle($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0.0, $10 = 0.0, $11 = 0.0, $12 = 0, $13 = 0.0, $2 = 0, $3 = 0.0, $4 = 0, $5 = 0.0, $6 = 0.0, $7 = 0.0, $8 = 0.0, $9 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ((($1)) + 4|0);
 $3 = +HEAPF32[$2>>2];
 $4 = ((($0)) + 4|0);
 $5 = +HEAPF32[$4>>2];
 $6 = $3 - $5;
 $7 = +HEAPF32[$1>>2];
 $8 = +HEAPF32[$0>>2];
 $9 = $7 - $8;
 $10 = (+Math_atan2((+$6),(+$9)));
 $11 = $10 * 57.2957763671875;
 $12 = $11 < 0.0;
 $13 = $11 + 360.0;
 $$0 = $12 ? $13 : $11;
 return (+$$0);
}
function _VectorZero($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 HEAPF32[$0>>2] = 0.0;
 $1 = ((($0)) + 4|0);
 HEAPF32[$1>>2] = 0.0;
 $2 = ((($0)) + 8|0);
 HEAPF32[$2>>2] = 0.0;
 return;
}
function _VectorLength($0) {
 $0 = $0|0;
 var $1 = 0.0, $10 = 0.0, $11 = 0.0, $2 = 0.0, $3 = 0, $4 = 0.0, $5 = 0.0, $6 = 0.0, $7 = 0, $8 = 0.0, $9 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = +HEAPF32[$0>>2];
 $2 = $1 * $1;
 $3 = ((($0)) + 4|0);
 $4 = +HEAPF32[$3>>2];
 $5 = $4 * $4;
 $6 = $2 + $5;
 $7 = ((($0)) + 8|0);
 $8 = +HEAPF32[$7>>2];
 $9 = $8 * $8;
 $10 = $6 + $9;
 $11 = (+Math_sqrt((+$10)));
 return (+$11);
}
function _VectorNormalize($0) {
 $0 = $0|0;
 var $$byval_copy = 0, $$op = 0.0, $1 = 0.0, $10 = 0.0, $11 = 0.0, $2 = 0, $3 = 0.0, $4 = 0.0, $5 = 0.0, $6 = 0, $7 = 0.0, $8 = 0.0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $$byval_copy = sp;
 ;HEAP32[$$byval_copy>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy+8>>2]=HEAP32[$0+8>>2]|0;
 $1 = (+_VectorLength($$byval_copy));
 $2 = $1 == 0.0;
 $$op = 1.0 / $1;
 $3 = $2 ? 1.0 : $$op;
 $4 = +HEAPF32[$0>>2];
 $5 = $4 * $3;
 HEAPF32[$0>>2] = $5;
 $6 = ((($0)) + 4|0);
 $7 = +HEAPF32[$6>>2];
 $8 = $3 * $7;
 HEAPF32[$6>>2] = $8;
 $9 = ((($0)) + 8|0);
 $10 = +HEAPF32[$9>>2];
 $11 = $3 * $10;
 HEAPF32[$9>>2] = $11;
 STACKTOP = sp;return;
}
function _VectorTransform($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $10 = 0.0, $11 = 0.0, $12 = 0.0, $13 = 0, $14 = 0.0, $15 = 0.0, $16 = 0.0, $17 = 0, $18 = 0.0, $19 = 0.0, $2 = 0.0, $20 = 0, $21 = 0.0, $22 = 0.0, $23 = 0, $24 = 0.0, $25 = 0.0, $26 = 0.0, $27 = 0, $28 = 0.0;
 var $29 = 0.0, $3 = 0, $30 = 0.0, $31 = 0, $32 = 0.0, $33 = 0.0, $34 = 0, $35 = 0.0, $36 = 0.0, $37 = 0, $38 = 0.0, $39 = 0.0, $4 = 0.0, $40 = 0.0, $41 = 0, $42 = 0.0, $43 = 0.0, $44 = 0.0, $45 = 0, $46 = 0.0;
 var $47 = 0.0, $5 = 0, $6 = 0.0, $7 = 0.0, $8 = 0.0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = +HEAPF32[$0>>2];
 $3 = ((($0)) + 4|0);
 $4 = +HEAPF32[$3>>2];
 $5 = ((($0)) + 8|0);
 $6 = +HEAPF32[$5>>2];
 $7 = +HEAPF32[$1>>2];
 $8 = $2 * $7;
 $9 = ((($1)) + 4|0);
 $10 = +HEAPF32[$9>>2];
 $11 = $4 * $10;
 $12 = $8 + $11;
 $13 = ((($1)) + 8|0);
 $14 = +HEAPF32[$13>>2];
 $15 = $6 * $14;
 $16 = $12 + $15;
 $17 = ((($1)) + 12|0);
 $18 = +HEAPF32[$17>>2];
 $19 = $18 + $16;
 HEAPF32[$0>>2] = $19;
 $20 = ((($1)) + 16|0);
 $21 = +HEAPF32[$20>>2];
 $22 = $2 * $21;
 $23 = ((($1)) + 20|0);
 $24 = +HEAPF32[$23>>2];
 $25 = $4 * $24;
 $26 = $22 + $25;
 $27 = ((($1)) + 24|0);
 $28 = +HEAPF32[$27>>2];
 $29 = $6 * $28;
 $30 = $26 + $29;
 $31 = ((($1)) + 28|0);
 $32 = +HEAPF32[$31>>2];
 $33 = $32 + $30;
 HEAPF32[$3>>2] = $33;
 $34 = ((($1)) + 32|0);
 $35 = +HEAPF32[$34>>2];
 $36 = $2 * $35;
 $37 = ((($1)) + 36|0);
 $38 = +HEAPF32[$37>>2];
 $39 = $4 * $38;
 $40 = $36 + $39;
 $41 = ((($1)) + 40|0);
 $42 = +HEAPF32[$41>>2];
 $43 = $6 * $42;
 $44 = $40 + $43;
 $45 = ((($1)) + 44|0);
 $46 = +HEAPF32[$45>>2];
 $47 = $46 + $44;
 HEAPF32[$5>>2] = $47;
 return;
}
function _MatrixTranspose($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $3 = 0, $4 = 0, $5 = 0;
 var $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ((($0)) + 4|0);
 $2 = HEAP32[$1>>2]|0;
 $3 = ((($0)) + 8|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ((($0)) + 12|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = ((($0)) + 16|0);
 $8 = HEAP32[$7>>2]|0;
 $9 = ((($0)) + 24|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = ((($0)) + 28|0);
 $12 = HEAP32[$11>>2]|0;
 $13 = ((($0)) + 32|0);
 $14 = HEAP32[$13>>2]|0;
 $15 = ((($0)) + 36|0);
 $16 = HEAP32[$15>>2]|0;
 $17 = ((($0)) + 44|0);
 $18 = HEAP32[$17>>2]|0;
 $19 = ((($0)) + 48|0);
 $20 = HEAP32[$19>>2]|0;
 $21 = ((($0)) + 52|0);
 $22 = HEAP32[$21>>2]|0;
 $23 = ((($0)) + 56|0);
 $24 = HEAP32[$23>>2]|0;
 HEAP32[$1>>2] = $8;
 HEAP32[$3>>2] = $14;
 HEAP32[$5>>2] = $20;
 HEAP32[$7>>2] = $2;
 HEAP32[$9>>2] = $16;
 HEAP32[$11>>2] = $22;
 HEAP32[$13>>2] = $4;
 HEAP32[$15>>2] = $10;
 HEAP32[$17>>2] = $24;
 HEAP32[$19>>2] = $6;
 HEAP32[$21>>2] = $12;
 HEAP32[$23>>2] = $18;
 return;
}
function _MatrixIdentity($0) {
 $0 = $0|0;
 var $$sroa$5$0$$sroa_idx = 0, $$sroa$55$0$$sroa_idx6 = 0, $$sroa$6$0$$sroa_idx = 0, $$sroa$611$0$$sroa_idx12 = 0, $$sroa$7$0$$sroa_idx = 0, $$sroa$717$0$$sroa_idx18 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 HEAPF32[$0>>2] = 1.0;
 $$sroa$5$0$$sroa_idx = ((($0)) + 4|0);
 ;HEAP32[$$sroa$5$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+12>>2]=0|0;
 $$sroa$55$0$$sroa_idx6 = ((($0)) + 20|0);
 HEAPF32[$$sroa$55$0$$sroa_idx6>>2] = 1.0;
 $$sroa$6$0$$sroa_idx = ((($0)) + 24|0);
 ;HEAP32[$$sroa$6$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+12>>2]=0|0;
 $$sroa$611$0$$sroa_idx12 = ((($0)) + 40|0);
 HEAPF32[$$sroa$611$0$$sroa_idx12>>2] = 1.0;
 $$sroa$7$0$$sroa_idx = ((($0)) + 44|0);
 ;HEAP32[$$sroa$7$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+12>>2]=0|0;
 $$sroa$717$0$$sroa_idx18 = ((($0)) + 60|0);
 HEAPF32[$$sroa$717$0$$sroa_idx18>>2] = 1.0;
 return;
}
function _MatrixTranslate($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = +$1;
 $2 = +$2;
 $3 = +$3;
 var $$sroa$13$0$$sroa_idx20 = 0, $$sroa$14$0$$sroa_idx22 = 0, $$sroa$15$0$$sroa_idx24 = 0, $$sroa$16$0$$sroa_idx26 = 0, $$sroa$17$0$$sroa_idx28 = 0, $$sroa$18$0$$sroa_idx30 = 0, $$sroa$4$0$$sroa_idx2 = 0, $$sroa$8$0$$sroa_idx10 = 0, $$sroa$9$0$$sroa_idx12 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 HEAPF32[$0>>2] = 1.0;
 $$sroa$4$0$$sroa_idx2 = ((($0)) + 4|0);
 $$sroa$8$0$$sroa_idx10 = ((($0)) + 20|0);
 ;HEAP32[$$sroa$4$0$$sroa_idx2>>2]=0|0;HEAP32[$$sroa$4$0$$sroa_idx2+4>>2]=0|0;HEAP32[$$sroa$4$0$$sroa_idx2+8>>2]=0|0;HEAP32[$$sroa$4$0$$sroa_idx2+12>>2]=0|0;
 HEAPF32[$$sroa$8$0$$sroa_idx10>>2] = 1.0;
 $$sroa$9$0$$sroa_idx12 = ((($0)) + 24|0);
 $$sroa$13$0$$sroa_idx20 = ((($0)) + 40|0);
 ;HEAP32[$$sroa$9$0$$sroa_idx12>>2]=0|0;HEAP32[$$sroa$9$0$$sroa_idx12+4>>2]=0|0;HEAP32[$$sroa$9$0$$sroa_idx12+8>>2]=0|0;HEAP32[$$sroa$9$0$$sroa_idx12+12>>2]=0|0;
 HEAPF32[$$sroa$13$0$$sroa_idx20>>2] = 1.0;
 $$sroa$14$0$$sroa_idx22 = ((($0)) + 44|0);
 HEAPF32[$$sroa$14$0$$sroa_idx22>>2] = 0.0;
 $$sroa$15$0$$sroa_idx24 = ((($0)) + 48|0);
 HEAPF32[$$sroa$15$0$$sroa_idx24>>2] = $1;
 $$sroa$16$0$$sroa_idx26 = ((($0)) + 52|0);
 HEAPF32[$$sroa$16$0$$sroa_idx26>>2] = $2;
 $$sroa$17$0$$sroa_idx28 = ((($0)) + 56|0);
 HEAPF32[$$sroa$17$0$$sroa_idx28>>2] = $3;
 $$sroa$18$0$$sroa_idx30 = ((($0)) + 60|0);
 HEAPF32[$$sroa$18$0$$sroa_idx30>>2] = 1.0;
 return;
}
function _MatrixRotate($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = +$2;
 var $$ = 0.0, $$221 = 0.0, $$222 = 0.0, $$sroa$10$0$$sroa_idx199 = 0, $$sroa$11$0$$sroa_idx201 = 0, $$sroa$12$0$$sroa_idx203 = 0, $$sroa$13$0$$sroa_idx205 = 0, $$sroa$14$0$$sroa_idx207 = 0, $$sroa$15$0$$sroa_idx209 = 0, $$sroa$16$0$$sroa_idx211 = 0, $$sroa$17$0$$sroa_idx213 = 0, $$sroa$18$0$$sroa_idx215 = 0, $$sroa$4$0$$sroa_idx187 = 0, $$sroa$5$0$$sroa_idx189 = 0, $$sroa$6$0$$sroa_idx191 = 0, $$sroa$7$0$$sroa_idx193 = 0, $$sroa$8$0$$sroa_idx195 = 0, $$sroa$9$0$$sroa_idx197 = 0, $10 = 0.0, $100 = 0.0;
 var $101 = 0.0, $102 = 0.0, $103 = 0.0, $104 = 0.0, $105 = 0.0, $106 = 0.0, $107 = 0.0, $108 = 0.0, $109 = 0.0, $11 = 0.0, $110 = 0.0, $111 = 0.0, $112 = 0.0, $113 = 0.0, $114 = 0.0, $115 = 0.0, $116 = 0.0, $117 = 0.0, $118 = 0.0, $119 = 0.0;
 var $12 = 0.0, $120 = 0.0, $121 = 0.0, $122 = 0.0, $123 = 0.0, $124 = 0.0, $125 = 0.0, $126 = 0.0, $127 = 0.0, $128 = 0.0, $129 = 0.0, $13 = 0.0, $130 = 0.0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0;
 var $138 = 0, $14 = 0.0, $15 = 0, $16 = 0, $17 = 0.0, $18 = 0.0, $19 = 0.0, $20 = 0.0, $21 = 0.0, $22 = 0.0, $23 = 0.0, $24 = 0.0, $25 = 0, $26 = 0.0, $27 = 0, $28 = 0.0, $29 = 0, $3 = 0, $30 = 0.0, $31 = 0;
 var $32 = 0.0, $33 = 0, $34 = 0.0, $35 = 0, $36 = 0.0, $37 = 0, $38 = 0.0, $39 = 0, $4 = 0.0, $40 = 0.0, $41 = 0, $42 = 0.0, $43 = 0, $44 = 0.0, $45 = 0, $46 = 0.0, $47 = 0.0, $48 = 0.0, $49 = 0.0, $5 = 0;
 var $50 = 0.0, $51 = 0.0, $52 = 0.0, $53 = 0.0, $54 = 0.0, $55 = 0.0, $56 = 0.0, $57 = 0.0, $58 = 0.0, $59 = 0.0, $6 = 0.0, $60 = 0.0, $61 = 0.0, $62 = 0.0, $63 = 0.0, $64 = 0.0, $65 = 0.0, $66 = 0.0, $67 = 0.0, $68 = 0.0;
 var $69 = 0.0, $7 = 0, $70 = 0.0, $71 = 0.0, $72 = 0.0, $73 = 0.0, $74 = 0.0, $75 = 0.0, $76 = 0.0, $77 = 0.0, $78 = 0.0, $79 = 0.0, $8 = 0.0, $80 = 0.0, $81 = 0.0, $82 = 0.0, $83 = 0.0, $84 = 0.0, $85 = 0.0, $86 = 0.0;
 var $87 = 0.0, $88 = 0.0, $89 = 0.0, $9 = 0.0, $90 = 0.0, $91 = 0.0, $92 = 0.0, $93 = 0.0, $94 = 0.0, $95 = 0.0, $96 = 0.0, $97 = 0.0, $98 = 0.0, $99 = 0.0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $3 = sp;
 _MatrixIdentity($3);
 $4 = +HEAPF32[$1>>2];
 $5 = ((($1)) + 4|0);
 $6 = +HEAPF32[$5>>2];
 $7 = ((($1)) + 8|0);
 $8 = +HEAPF32[$7>>2];
 $9 = $4 * $4;
 $10 = $6 * $6;
 $11 = $9 + $10;
 $12 = $8 * $8;
 $13 = $11 + $12;
 $14 = (+Math_sqrt((+$13)));
 $15 = $14 != 1.0;
 $16 = $14 != 0.0;
 $or$cond = $15 & $16;
 $17 = 1.0 / $14;
 $18 = $4 * $17;
 $19 = $6 * $17;
 $20 = $8 * $17;
 $$ = $or$cond ? $20 : $8;
 $$221 = $or$cond ? $19 : $6;
 $$222 = $or$cond ? $18 : $4;
 $21 = (+Math_sin((+$2)));
 $22 = (+Math_cos((+$2)));
 $23 = 1.0 - $22;
 $24 = +HEAPF32[$3>>2];
 $25 = ((($3)) + 16|0);
 $26 = +HEAPF32[$25>>2];
 $27 = ((($3)) + 32|0);
 $28 = +HEAPF32[$27>>2];
 $29 = ((($3)) + 48|0);
 $30 = +HEAPF32[$29>>2];
 $31 = ((($3)) + 4|0);
 $32 = +HEAPF32[$31>>2];
 $33 = ((($3)) + 20|0);
 $34 = +HEAPF32[$33>>2];
 $35 = ((($3)) + 36|0);
 $36 = +HEAPF32[$35>>2];
 $37 = ((($3)) + 52|0);
 $38 = +HEAPF32[$37>>2];
 $39 = ((($3)) + 8|0);
 $40 = +HEAPF32[$39>>2];
 $41 = ((($3)) + 24|0);
 $42 = +HEAPF32[$41>>2];
 $43 = ((($3)) + 40|0);
 $44 = +HEAPF32[$43>>2];
 $45 = ((($3)) + 56|0);
 $46 = +HEAPF32[$45>>2];
 $47 = $$222 * $$222;
 $48 = $23 * $47;
 $49 = $22 + $48;
 $50 = $$221 * $$222;
 $51 = $23 * $50;
 $52 = $21 * $$;
 $53 = $52 + $51;
 $54 = $$ * $$222;
 $55 = $23 * $54;
 $56 = $21 * $$221;
 $57 = $55 - $56;
 $58 = $51 - $52;
 $59 = $$221 * $$221;
 $60 = $23 * $59;
 $61 = $22 + $60;
 $62 = $$ * $$221;
 $63 = $23 * $62;
 $64 = $21 * $$222;
 $65 = $64 + $63;
 $66 = $56 + $55;
 $67 = $63 - $64;
 $68 = $$ * $$;
 $69 = $23 * $68;
 $70 = $22 + $69;
 $71 = $24 * $49;
 $72 = $53 * $32;
 $73 = $71 + $72;
 $74 = $57 * $40;
 $75 = $73 + $74;
 $76 = $26 * $49;
 $77 = $53 * $34;
 $78 = $76 + $77;
 $79 = $57 * $42;
 $80 = $78 + $79;
 $81 = $28 * $49;
 $82 = $53 * $36;
 $83 = $81 + $82;
 $84 = $57 * $44;
 $85 = $83 + $84;
 $86 = $30 * $49;
 $87 = $53 * $38;
 $88 = $86 + $87;
 $89 = $57 * $46;
 $90 = $88 + $89;
 $91 = $24 * $58;
 $92 = $61 * $32;
 $93 = $91 + $92;
 $94 = $65 * $40;
 $95 = $93 + $94;
 $96 = $26 * $58;
 $97 = $61 * $34;
 $98 = $96 + $97;
 $99 = $65 * $42;
 $100 = $98 + $99;
 $101 = $28 * $58;
 $102 = $61 * $36;
 $103 = $101 + $102;
 $104 = $65 * $44;
 $105 = $103 + $104;
 $106 = $30 * $58;
 $107 = $61 * $38;
 $108 = $106 + $107;
 $109 = $65 * $46;
 $110 = $108 + $109;
 $111 = $24 * $66;
 $112 = $67 * $32;
 $113 = $111 + $112;
 $114 = $70 * $40;
 $115 = $113 + $114;
 $116 = $26 * $66;
 $117 = $67 * $34;
 $118 = $116 + $117;
 $119 = $70 * $42;
 $120 = $118 + $119;
 $121 = $28 * $66;
 $122 = $67 * $36;
 $123 = $121 + $122;
 $124 = $70 * $44;
 $125 = $123 + $124;
 $126 = $30 * $66;
 $127 = $67 * $38;
 $128 = $126 + $127;
 $129 = $70 * $46;
 $130 = $128 + $129;
 $131 = ((($3)) + 12|0);
 $132 = HEAP32[$131>>2]|0;
 $133 = ((($3)) + 28|0);
 $134 = HEAP32[$133>>2]|0;
 $135 = ((($3)) + 44|0);
 $136 = HEAP32[$135>>2]|0;
 $137 = ((($3)) + 60|0);
 $138 = HEAP32[$137>>2]|0;
 HEAPF32[$0>>2] = $75;
 $$sroa$4$0$$sroa_idx187 = ((($0)) + 4|0);
 HEAPF32[$$sroa$4$0$$sroa_idx187>>2] = $95;
 $$sroa$5$0$$sroa_idx189 = ((($0)) + 8|0);
 HEAPF32[$$sroa$5$0$$sroa_idx189>>2] = $115;
 $$sroa$6$0$$sroa_idx191 = ((($0)) + 12|0);
 HEAP32[$$sroa$6$0$$sroa_idx191>>2] = $132;
 $$sroa$7$0$$sroa_idx193 = ((($0)) + 16|0);
 HEAPF32[$$sroa$7$0$$sroa_idx193>>2] = $80;
 $$sroa$8$0$$sroa_idx195 = ((($0)) + 20|0);
 HEAPF32[$$sroa$8$0$$sroa_idx195>>2] = $100;
 $$sroa$9$0$$sroa_idx197 = ((($0)) + 24|0);
 HEAPF32[$$sroa$9$0$$sroa_idx197>>2] = $120;
 $$sroa$10$0$$sroa_idx199 = ((($0)) + 28|0);
 HEAP32[$$sroa$10$0$$sroa_idx199>>2] = $134;
 $$sroa$11$0$$sroa_idx201 = ((($0)) + 32|0);
 HEAPF32[$$sroa$11$0$$sroa_idx201>>2] = $85;
 $$sroa$12$0$$sroa_idx203 = ((($0)) + 36|0);
 HEAPF32[$$sroa$12$0$$sroa_idx203>>2] = $105;
 $$sroa$13$0$$sroa_idx205 = ((($0)) + 40|0);
 HEAPF32[$$sroa$13$0$$sroa_idx205>>2] = $125;
 $$sroa$14$0$$sroa_idx207 = ((($0)) + 44|0);
 HEAP32[$$sroa$14$0$$sroa_idx207>>2] = $136;
 $$sroa$15$0$$sroa_idx209 = ((($0)) + 48|0);
 HEAPF32[$$sroa$15$0$$sroa_idx209>>2] = $90;
 $$sroa$16$0$$sroa_idx211 = ((($0)) + 52|0);
 HEAPF32[$$sroa$16$0$$sroa_idx211>>2] = $110;
 $$sroa$17$0$$sroa_idx213 = ((($0)) + 56|0);
 HEAPF32[$$sroa$17$0$$sroa_idx213>>2] = $130;
 $$sroa$18$0$$sroa_idx215 = ((($0)) + 60|0);
 HEAP32[$$sroa$18$0$$sroa_idx215>>2] = $138;
 STACKTOP = sp;return;
}
function _MatrixScale($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = +$1;
 $2 = +$2;
 $3 = +$3;
 var $$sroa$5$0$$sroa_idx = 0, $$sroa$55$0$$sroa_idx6 = 0, $$sroa$6$0$$sroa_idx = 0, $$sroa$611$0$$sroa_idx12 = 0, $$sroa$7$0$$sroa_idx = 0, $$sroa$717$0$$sroa_idx18 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 HEAPF32[$0>>2] = $1;
 $$sroa$5$0$$sroa_idx = ((($0)) + 4|0);
 ;HEAP32[$$sroa$5$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$5$0$$sroa_idx+12>>2]=0|0;
 $$sroa$55$0$$sroa_idx6 = ((($0)) + 20|0);
 HEAPF32[$$sroa$55$0$$sroa_idx6>>2] = $2;
 $$sroa$6$0$$sroa_idx = ((($0)) + 24|0);
 ;HEAP32[$$sroa$6$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$6$0$$sroa_idx+12>>2]=0|0;
 $$sroa$611$0$$sroa_idx12 = ((($0)) + 40|0);
 HEAPF32[$$sroa$611$0$$sroa_idx12>>2] = $3;
 $$sroa$7$0$$sroa_idx = ((($0)) + 44|0);
 ;HEAP32[$$sroa$7$0$$sroa_idx>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+4>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+8>>2]=0|0;HEAP32[$$sroa$7$0$$sroa_idx+12>>2]=0|0;
 $$sroa$717$0$$sroa_idx18 = ((($0)) + 60|0);
 HEAPF32[$$sroa$717$0$$sroa_idx18>>2] = 1.0;
 return;
}
function _MatrixMultiply($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$sroa$10$0$$sroa_idx14 = 0, $$sroa$11$0$$sroa_idx16 = 0, $$sroa$12$0$$sroa_idx18 = 0, $$sroa$13$0$$sroa_idx20 = 0, $$sroa$14$0$$sroa_idx22 = 0, $$sroa$15$0$$sroa_idx24 = 0, $$sroa$16$0$$sroa_idx26 = 0, $$sroa$17$0$$sroa_idx28 = 0, $$sroa$18$0$$sroa_idx30 = 0, $$sroa$4$0$$sroa_idx2 = 0, $$sroa$5$0$$sroa_idx4 = 0, $$sroa$6$0$$sroa_idx6 = 0, $$sroa$7$0$$sroa_idx8 = 0, $$sroa$8$0$$sroa_idx10 = 0, $$sroa$9$0$$sroa_idx12 = 0, $10 = 0.0, $100 = 0.0, $101 = 0.0, $102 = 0.0, $103 = 0.0;
 var $104 = 0.0, $105 = 0, $106 = 0.0, $107 = 0.0, $108 = 0, $109 = 0.0, $11 = 0.0, $110 = 0.0, $111 = 0.0, $112 = 0, $113 = 0.0, $114 = 0.0, $115 = 0.0, $116 = 0, $117 = 0.0, $118 = 0.0, $119 = 0.0, $12 = 0, $120 = 0.0, $121 = 0.0;
 var $122 = 0.0, $123 = 0.0, $124 = 0.0, $125 = 0.0, $126 = 0.0, $127 = 0.0, $128 = 0.0, $129 = 0.0, $13 = 0.0, $130 = 0.0, $131 = 0.0, $132 = 0.0, $133 = 0.0, $134 = 0.0, $135 = 0.0, $136 = 0.0, $137 = 0.0, $138 = 0.0, $139 = 0.0, $14 = 0;
 var $140 = 0.0, $141 = 0, $142 = 0.0, $143 = 0.0, $144 = 0, $145 = 0.0, $146 = 0.0, $147 = 0.0, $148 = 0, $149 = 0.0, $15 = 0.0, $150 = 0.0, $151 = 0.0, $152 = 0, $153 = 0.0, $154 = 0.0, $155 = 0.0, $156 = 0.0, $157 = 0.0, $158 = 0.0;
 var $159 = 0.0, $16 = 0.0, $160 = 0.0, $161 = 0.0, $162 = 0.0, $163 = 0.0, $164 = 0.0, $165 = 0.0, $166 = 0.0, $167 = 0.0, $168 = 0.0, $169 = 0.0, $17 = 0.0, $170 = 0.0, $171 = 0.0, $172 = 0.0, $173 = 0.0, $174 = 0.0, $175 = 0.0, $176 = 0.0;
 var $18 = 0, $19 = 0.0, $20 = 0, $21 = 0.0, $22 = 0.0, $23 = 0.0, $24 = 0, $25 = 0.0, $26 = 0.0, $27 = 0, $28 = 0.0, $29 = 0.0, $3 = 0.0, $30 = 0.0, $31 = 0, $32 = 0.0, $33 = 0.0, $34 = 0.0, $35 = 0, $36 = 0.0;
 var $37 = 0.0, $38 = 0.0, $39 = 0, $4 = 0.0, $40 = 0.0, $41 = 0.0, $42 = 0, $43 = 0.0, $44 = 0.0, $45 = 0.0, $46 = 0, $47 = 0.0, $48 = 0.0, $49 = 0.0, $5 = 0.0, $50 = 0, $51 = 0.0, $52 = 0.0, $53 = 0.0, $54 = 0;
 var $55 = 0.0, $56 = 0.0, $57 = 0, $58 = 0.0, $59 = 0.0, $6 = 0, $60 = 0.0, $61 = 0, $62 = 0.0, $63 = 0.0, $64 = 0.0, $65 = 0, $66 = 0.0, $67 = 0.0, $68 = 0.0, $69 = 0, $7 = 0.0, $70 = 0.0, $71 = 0.0, $72 = 0;
 var $73 = 0.0, $74 = 0.0, $75 = 0.0, $76 = 0, $77 = 0.0, $78 = 0.0, $79 = 0.0, $8 = 0, $80 = 0, $81 = 0.0, $82 = 0.0, $83 = 0.0, $84 = 0.0, $85 = 0.0, $86 = 0.0, $87 = 0.0, $88 = 0.0, $89 = 0.0, $9 = 0.0, $90 = 0.0;
 var $91 = 0.0, $92 = 0.0, $93 = 0.0, $94 = 0.0, $95 = 0.0, $96 = 0.0, $97 = 0.0, $98 = 0.0, $99 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = +HEAPF32[$2>>2];
 $4 = +HEAPF32[$1>>2];
 $5 = $3 * $4;
 $6 = ((($2)) + 16|0);
 $7 = +HEAPF32[$6>>2];
 $8 = ((($1)) + 4|0);
 $9 = +HEAPF32[$8>>2];
 $10 = $7 * $9;
 $11 = $5 + $10;
 $12 = ((($2)) + 32|0);
 $13 = +HEAPF32[$12>>2];
 $14 = ((($1)) + 8|0);
 $15 = +HEAPF32[$14>>2];
 $16 = $13 * $15;
 $17 = $11 + $16;
 $18 = ((($2)) + 48|0);
 $19 = +HEAPF32[$18>>2];
 $20 = ((($1)) + 12|0);
 $21 = +HEAPF32[$20>>2];
 $22 = $19 * $21;
 $23 = $17 + $22;
 $24 = ((($1)) + 16|0);
 $25 = +HEAPF32[$24>>2];
 $26 = $3 * $25;
 $27 = ((($1)) + 20|0);
 $28 = +HEAPF32[$27>>2];
 $29 = $7 * $28;
 $30 = $26 + $29;
 $31 = ((($1)) + 24|0);
 $32 = +HEAPF32[$31>>2];
 $33 = $13 * $32;
 $34 = $30 + $33;
 $35 = ((($1)) + 28|0);
 $36 = +HEAPF32[$35>>2];
 $37 = $19 * $36;
 $38 = $34 + $37;
 $39 = ((($1)) + 32|0);
 $40 = +HEAPF32[$39>>2];
 $41 = $3 * $40;
 $42 = ((($1)) + 36|0);
 $43 = +HEAPF32[$42>>2];
 $44 = $7 * $43;
 $45 = $41 + $44;
 $46 = ((($1)) + 40|0);
 $47 = +HEAPF32[$46>>2];
 $48 = $13 * $47;
 $49 = $45 + $48;
 $50 = ((($1)) + 44|0);
 $51 = +HEAPF32[$50>>2];
 $52 = $19 * $51;
 $53 = $49 + $52;
 $54 = ((($1)) + 48|0);
 $55 = +HEAPF32[$54>>2];
 $56 = $3 * $55;
 $57 = ((($1)) + 52|0);
 $58 = +HEAPF32[$57>>2];
 $59 = $7 * $58;
 $60 = $56 + $59;
 $61 = ((($1)) + 56|0);
 $62 = +HEAPF32[$61>>2];
 $63 = $13 * $62;
 $64 = $60 + $63;
 $65 = ((($1)) + 60|0);
 $66 = +HEAPF32[$65>>2];
 $67 = $19 * $66;
 $68 = $64 + $67;
 $69 = ((($2)) + 4|0);
 $70 = +HEAPF32[$69>>2];
 $71 = $4 * $70;
 $72 = ((($2)) + 20|0);
 $73 = +HEAPF32[$72>>2];
 $74 = $9 * $73;
 $75 = $71 + $74;
 $76 = ((($2)) + 36|0);
 $77 = +HEAPF32[$76>>2];
 $78 = $15 * $77;
 $79 = $75 + $78;
 $80 = ((($2)) + 52|0);
 $81 = +HEAPF32[$80>>2];
 $82 = $21 * $81;
 $83 = $79 + $82;
 $84 = $25 * $70;
 $85 = $28 * $73;
 $86 = $84 + $85;
 $87 = $32 * $77;
 $88 = $86 + $87;
 $89 = $36 * $81;
 $90 = $88 + $89;
 $91 = $40 * $70;
 $92 = $43 * $73;
 $93 = $91 + $92;
 $94 = $47 * $77;
 $95 = $93 + $94;
 $96 = $51 * $81;
 $97 = $95 + $96;
 $98 = $55 * $70;
 $99 = $58 * $73;
 $100 = $98 + $99;
 $101 = $62 * $77;
 $102 = $100 + $101;
 $103 = $66 * $81;
 $104 = $102 + $103;
 $105 = ((($2)) + 8|0);
 $106 = +HEAPF32[$105>>2];
 $107 = $4 * $106;
 $108 = ((($2)) + 24|0);
 $109 = +HEAPF32[$108>>2];
 $110 = $9 * $109;
 $111 = $107 + $110;
 $112 = ((($2)) + 40|0);
 $113 = +HEAPF32[$112>>2];
 $114 = $15 * $113;
 $115 = $111 + $114;
 $116 = ((($2)) + 56|0);
 $117 = +HEAPF32[$116>>2];
 $118 = $21 * $117;
 $119 = $115 + $118;
 $120 = $25 * $106;
 $121 = $28 * $109;
 $122 = $120 + $121;
 $123 = $32 * $113;
 $124 = $122 + $123;
 $125 = $36 * $117;
 $126 = $124 + $125;
 $127 = $40 * $106;
 $128 = $43 * $109;
 $129 = $127 + $128;
 $130 = $47 * $113;
 $131 = $129 + $130;
 $132 = $51 * $117;
 $133 = $131 + $132;
 $134 = $55 * $106;
 $135 = $58 * $109;
 $136 = $134 + $135;
 $137 = $62 * $113;
 $138 = $136 + $137;
 $139 = $66 * $117;
 $140 = $138 + $139;
 $141 = ((($2)) + 12|0);
 $142 = +HEAPF32[$141>>2];
 $143 = $4 * $142;
 $144 = ((($2)) + 28|0);
 $145 = +HEAPF32[$144>>2];
 $146 = $9 * $145;
 $147 = $143 + $146;
 $148 = ((($2)) + 44|0);
 $149 = +HEAPF32[$148>>2];
 $150 = $15 * $149;
 $151 = $147 + $150;
 $152 = ((($2)) + 60|0);
 $153 = +HEAPF32[$152>>2];
 $154 = $21 * $153;
 $155 = $151 + $154;
 $156 = $25 * $142;
 $157 = $28 * $145;
 $158 = $156 + $157;
 $159 = $32 * $149;
 $160 = $158 + $159;
 $161 = $36 * $153;
 $162 = $160 + $161;
 $163 = $40 * $142;
 $164 = $43 * $145;
 $165 = $163 + $164;
 $166 = $47 * $149;
 $167 = $165 + $166;
 $168 = $51 * $153;
 $169 = $167 + $168;
 $170 = $55 * $142;
 $171 = $58 * $145;
 $172 = $170 + $171;
 $173 = $62 * $149;
 $174 = $172 + $173;
 $175 = $66 * $153;
 $176 = $174 + $175;
 HEAPF32[$0>>2] = $23;
 $$sroa$4$0$$sroa_idx2 = ((($0)) + 4|0);
 HEAPF32[$$sroa$4$0$$sroa_idx2>>2] = $83;
 $$sroa$5$0$$sroa_idx4 = ((($0)) + 8|0);
 HEAPF32[$$sroa$5$0$$sroa_idx4>>2] = $119;
 $$sroa$6$0$$sroa_idx6 = ((($0)) + 12|0);
 HEAPF32[$$sroa$6$0$$sroa_idx6>>2] = $155;
 $$sroa$7$0$$sroa_idx8 = ((($0)) + 16|0);
 HEAPF32[$$sroa$7$0$$sroa_idx8>>2] = $38;
 $$sroa$8$0$$sroa_idx10 = ((($0)) + 20|0);
 HEAPF32[$$sroa$8$0$$sroa_idx10>>2] = $90;
 $$sroa$9$0$$sroa_idx12 = ((($0)) + 24|0);
 HEAPF32[$$sroa$9$0$$sroa_idx12>>2] = $126;
 $$sroa$10$0$$sroa_idx14 = ((($0)) + 28|0);
 HEAPF32[$$sroa$10$0$$sroa_idx14>>2] = $162;
 $$sroa$11$0$$sroa_idx16 = ((($0)) + 32|0);
 HEAPF32[$$sroa$11$0$$sroa_idx16>>2] = $53;
 $$sroa$12$0$$sroa_idx18 = ((($0)) + 36|0);
 HEAPF32[$$sroa$12$0$$sroa_idx18>>2] = $97;
 $$sroa$13$0$$sroa_idx20 = ((($0)) + 40|0);
 HEAPF32[$$sroa$13$0$$sroa_idx20>>2] = $133;
 $$sroa$14$0$$sroa_idx22 = ((($0)) + 44|0);
 HEAPF32[$$sroa$14$0$$sroa_idx22>>2] = $169;
 $$sroa$15$0$$sroa_idx24 = ((($0)) + 48|0);
 HEAPF32[$$sroa$15$0$$sroa_idx24>>2] = $68;
 $$sroa$16$0$$sroa_idx26 = ((($0)) + 52|0);
 HEAPF32[$$sroa$16$0$$sroa_idx26>>2] = $104;
 $$sroa$17$0$$sroa_idx28 = ((($0)) + 56|0);
 HEAPF32[$$sroa$17$0$$sroa_idx28>>2] = $140;
 $$sroa$18$0$$sroa_idx30 = ((($0)) + 60|0);
 HEAPF32[$$sroa$18$0$$sroa_idx30>>2] = $176;
 return;
}
function _MatrixOrtho($0,$1,$2,$3,$4,$5,$6) {
 $0 = $0|0;
 $1 = +$1;
 $2 = +$2;
 $3 = +$3;
 $4 = +$4;
 $5 = +$5;
 $6 = +$6;
 var $$sroa$10$0$$sroa_idx24 = 0, $$sroa$11$0$$sroa_idx26 = 0, $$sroa$12$0$$sroa_idx28 = 0, $$sroa$13$0$$sroa_idx30 = 0, $$sroa$14$0$$sroa_idx32 = 0, $$sroa$15$0$$sroa_idx34 = 0, $$sroa$16$0$$sroa_idx36 = 0, $$sroa$17$0$$sroa_idx38 = 0, $$sroa$18$0$$sroa_idx40 = 0, $$sroa$4$0$$sroa_idx12 = 0, $$sroa$5$0$$sroa_idx14 = 0, $$sroa$6$0$$sroa_idx16 = 0, $$sroa$7$0$$sroa_idx18 = 0, $$sroa$8$0$$sroa_idx20 = 0, $$sroa$9$0$$sroa_idx22 = 0, $10 = 0.0, $11 = 0.0, $12 = 0.0, $13 = 0.0, $14 = 0.0;
 var $15 = 0.0, $16 = 0.0, $17 = 0.0, $18 = 0.0, $19 = 0.0, $20 = 0.0, $21 = 0.0, $22 = 0.0, $23 = 0.0, $24 = 0.0, $25 = 0.0, $26 = 0.0, $27 = 0.0, $28 = 0.0, $29 = 0.0, $30 = 0.0, $7 = 0.0, $8 = 0.0, $9 = 0.0, label = 0;
 var sp = 0;
 sp = STACKTOP;
 $7 = $2 - $1;
 $8 = $7;
 $9 = $4 - $3;
 $10 = $9;
 $11 = $6 - $5;
 $12 = $11;
 $13 = 2.0 / $8;
 $14 = 2.0 / $10;
 $15 = -2.0 / $12;
 $16 = $1 + $2;
 $17 = -$16;
 $18 = $8;
 $19 = $17 / $18;
 $20 = $19;
 $21 = $3 + $4;
 $22 = -$21;
 $23 = $10;
 $24 = $22 / $23;
 $25 = $24;
 $26 = $5 + $6;
 $27 = -$26;
 $28 = $12;
 $29 = $27 / $28;
 $30 = $29;
 HEAPF32[$0>>2] = $13;
 $$sroa$4$0$$sroa_idx12 = ((($0)) + 4|0);
 HEAPF32[$$sroa$4$0$$sroa_idx12>>2] = 0.0;
 $$sroa$5$0$$sroa_idx14 = ((($0)) + 8|0);
 HEAPF32[$$sroa$5$0$$sroa_idx14>>2] = 0.0;
 $$sroa$6$0$$sroa_idx16 = ((($0)) + 12|0);
 HEAPF32[$$sroa$6$0$$sroa_idx16>>2] = $20;
 $$sroa$7$0$$sroa_idx18 = ((($0)) + 16|0);
 HEAPF32[$$sroa$7$0$$sroa_idx18>>2] = 0.0;
 $$sroa$8$0$$sroa_idx20 = ((($0)) + 20|0);
 HEAPF32[$$sroa$8$0$$sroa_idx20>>2] = $14;
 $$sroa$9$0$$sroa_idx22 = ((($0)) + 24|0);
 HEAPF32[$$sroa$9$0$$sroa_idx22>>2] = 0.0;
 $$sroa$10$0$$sroa_idx24 = ((($0)) + 28|0);
 HEAPF32[$$sroa$10$0$$sroa_idx24>>2] = $25;
 $$sroa$11$0$$sroa_idx26 = ((($0)) + 32|0);
 HEAPF32[$$sroa$11$0$$sroa_idx26>>2] = 0.0;
 $$sroa$12$0$$sroa_idx28 = ((($0)) + 36|0);
 HEAPF32[$$sroa$12$0$$sroa_idx28>>2] = 0.0;
 $$sroa$13$0$$sroa_idx30 = ((($0)) + 40|0);
 HEAPF32[$$sroa$13$0$$sroa_idx30>>2] = $15;
 $$sroa$14$0$$sroa_idx32 = ((($0)) + 44|0);
 HEAPF32[$$sroa$14$0$$sroa_idx32>>2] = $30;
 $$sroa$15$0$$sroa_idx34 = ((($0)) + 48|0);
 HEAPF32[$$sroa$15$0$$sroa_idx34>>2] = 0.0;
 $$sroa$16$0$$sroa_idx36 = ((($0)) + 52|0);
 HEAPF32[$$sroa$16$0$$sroa_idx36>>2] = 0.0;
 $$sroa$17$0$$sroa_idx38 = ((($0)) + 56|0);
 HEAPF32[$$sroa$17$0$$sroa_idx38>>2] = 0.0;
 $$sroa$18$0$$sroa_idx40 = ((($0)) + 60|0);
 HEAPF32[$$sroa$18$0$$sroa_idx40>>2] = 1.0;
 return;
}
function _ProcessGestureEvent($0) {
 $0 = $0|0;
 var $$$sink = 0, $$sink = 0, $$sink10 = 0, $$sink11 = 0, $$sink16 = 0, $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0.0, $111 = 0.0;
 var $112 = 0.0, $113 = 0.0, $114 = 0.0, $115 = 0.0, $116 = 0.0, $117 = 0, $118 = 0, $119 = 0, $12 = 0.0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0;
 var $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0;
 var $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0.0, $16 = 0, $160 = 0.0, $161 = 0.0, $162 = 0.0, $163 = 0.0, $164 = 0.0, $165 = 0.0, $166 = 0;
 var $167 = 0.0, $168 = 0, $169 = 0.0, $17 = 0, $170 = 0.0, $171 = 0.0, $172 = 0, $173 = 0.0, $174 = 0.0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0;
 var $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0;
 var $46 = 0, $47 = 0, $48 = 0.0, $49 = 0.0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0.0, $56 = 0.0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0;
 var $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0.0, $81 = 0;
 var $82 = 0.0, $83 = 0.0, $84 = 0.0, $85 = 0.0, $86 = 0.0, $87 = 0.0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $moveDownPosition$byval_copy11 = 0;
 var $moveDownPosition2$byval_copy12 = 0, $or$cond = 0, $or$cond3 = 0, $or$cond5 = 0, $or$cond7 = 0, $or$cond9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $moveDownPosition2$byval_copy12 = sp + 8|0;
 $moveDownPosition$byval_copy11 = sp;
 $1 = ((($0)) + 4|0);
 $2 = HEAP32[$1>>2]|0;
 HEAP32[3523] = $2;
 $3 = ($2|0)<(2);
 $4 = HEAP32[$0>>2]|0;
 $5 = ($4|0)==(1);
 if (!($3)) {
  if ($5) {
   $88 = ((($0)) + 24|0);
   $89 = $88;
   $90 = $89;
   $91 = HEAP32[$90>>2]|0;
   $92 = (($89) + 4)|0;
   $93 = $92;
   $94 = HEAP32[$93>>2]|0;
   $95 = 13816;
   $96 = $95;
   HEAP32[$96>>2] = $91;
   $97 = (($95) + 4)|0;
   $98 = $97;
   HEAP32[$98>>2] = $94;
   $99 = ((($0)) + 32|0);
   $100 = $99;
   $101 = $100;
   $102 = HEAP32[$101>>2]|0;
   $103 = (($100) + 4)|0;
   $104 = $103;
   $105 = HEAP32[$104>>2]|0;
   $106 = 13856;
   $107 = $106;
   HEAP32[$107>>2] = $102;
   $108 = (($106) + 4)|0;
   $109 = $108;
   HEAP32[$109>>2] = $105;
   $110 = +HEAPF32[3464];
   $111 = +HEAPF32[3454];
   $112 = $110 - $111;
   HEAPF32[3466] = $112;
   $113 = +HEAPF32[(13860)>>2];
   $114 = +HEAPF32[(13820)>>2];
   $115 = $113 - $114;
   HEAPF32[(13868)>>2] = $115;
   HEAP32[3522] = 4;
   STACKTOP = sp;return;
  }
  switch ($4|0) {
  case 2:  {
   ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13848>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13848+4>>2]|0;
   ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13872>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13872+4>>2]|0;
   $116 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
   HEAPF32[3528] = $116;
   $117 = 13848;
   $118 = $117;
   $119 = HEAP32[$118>>2]|0;
   $120 = (($117) + 4)|0;
   $121 = $120;
   $122 = HEAP32[$121>>2]|0;
   $123 = 13816;
   $124 = $123;
   HEAP32[$124>>2] = $119;
   $125 = (($123) + 4)|0;
   $126 = $125;
   HEAP32[$126>>2] = $122;
   $127 = 13872;
   $128 = $127;
   $129 = HEAP32[$128>>2]|0;
   $130 = (($127) + 4)|0;
   $131 = $130;
   $132 = HEAP32[$131>>2]|0;
   $133 = 13856;
   $134 = $133;
   HEAP32[$134>>2] = $129;
   $135 = (($133) + 4)|0;
   $136 = $135;
   HEAP32[$136>>2] = $132;
   $137 = ((($0)) + 24|0);
   $138 = $137;
   $139 = $138;
   $140 = HEAP32[$139>>2]|0;
   $141 = (($138) + 4)|0;
   $142 = $141;
   $143 = HEAP32[$142>>2]|0;
   $144 = 13848;
   $145 = $144;
   HEAP32[$145>>2] = $140;
   $146 = (($144) + 4)|0;
   $147 = $146;
   HEAP32[$147>>2] = $143;
   $148 = ((($0)) + 32|0);
   $149 = $148;
   $150 = $149;
   $151 = HEAP32[$150>>2]|0;
   $152 = (($149) + 4)|0;
   $153 = $152;
   $154 = HEAP32[$153>>2]|0;
   $155 = 13872;
   $156 = $155;
   HEAP32[$156>>2] = $151;
   $157 = (($155) + 4)|0;
   $158 = $157;
   HEAP32[$158>>2] = $154;
   $159 = +HEAPF32[3468];
   $160 = +HEAPF32[3462];
   $161 = $159 - $160;
   HEAPF32[3466] = $161;
   $162 = +HEAPF32[(13876)>>2];
   $163 = +HEAPF32[(13852)>>2];
   $164 = $162 - $163;
   HEAPF32[(13868)>>2] = $164;
   ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13816>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13816+4>>2]|0;
   ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13848>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13848+4>>2]|0;
   $165 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
   $166 = !($165 >= 0.004999999888241291);
   if ($166) {
    ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13856>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13856+4>>2]|0;
    ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13872>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13872+4>>2]|0;
    $167 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
    $168 = !($167 >= 0.004999999888241291);
    if ($168) {
     $$sink16 = 4;
    } else {
     label = 29;
    }
   } else {
    label = 29;
   }
   if ((label|0) == 29) {
    ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13848>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13848+4>>2]|0;
    ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13872>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13872+4>>2]|0;
    $169 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
    $170 = +HEAPF32[3528];
    $171 = $169 - $170;
    $172 = $171 < 0.0;
    $$sink11 = $172 ? 256 : 512;
    $$sink16 = $$sink11;
   }
   HEAP32[3522] = $$sink16;
   ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13848>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13848+4>>2]|0;
   ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13872>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13872+4>>2]|0;
   $173 = (+_Vector2Angle($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
   $174 = 360.0 - $173;
   HEAPF32[3529] = $174;
   STACKTOP = sp;return;
   break;
  }
  case 0:  {
   HEAPF32[3528] = 0.0;
   HEAPF32[3529] = 0.0;
   HEAPF32[3466] = 0.0;
   HEAPF32[(13868)>>2] = 0.0;
   HEAP32[3523] = 0;
   HEAP32[3522] = 0;
   STACKTOP = sp;return;
   break;
  }
  default: {
   STACKTOP = sp;return;
  }
  }
 }
 if ($5) {
  $6 = HEAP32[3524]|0;
  $7 = (($6) + 1)|0;
  HEAP32[3524] = $7;
  $8 = HEAP32[3522]|0;
  $9 = ($8|0)==(0);
  $10 = ($6|0)>(0);
  $or$cond = $10 & $9;
  if ($or$cond) {
   $11 = ((($0)) + 24|0);
   ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13816>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13816+4>>2]|0;
   ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[$11>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[$11+4>>2]|0;
   $12 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
   $13 = $12 < 0.029999999329447746;
   if ($13) {
    HEAP32[3522] = 2;
    HEAP32[3524] = 0;
   } else {
    label = 6;
   }
  } else {
   label = 6;
  }
  if ((label|0) == 6) {
   HEAP32[3524] = 1;
   HEAP32[3522] = 1;
  }
  $14 = ((($0)) + 24|0);
  $15 = $14;
  $16 = $15;
  $17 = HEAP32[$16>>2]|0;
  $18 = (($15) + 4)|0;
  $19 = $18;
  $20 = HEAP32[$19>>2]|0;
  $21 = 13816;
  $22 = $21;
  HEAP32[$22>>2] = $17;
  $23 = (($21) + 4)|0;
  $24 = $23;
  HEAP32[$24>>2] = $20;
  $25 = 13824;
  $26 = $25;
  HEAP32[$26>>2] = $17;
  $27 = (($25) + 4)|0;
  $28 = $27;
  HEAP32[$28>>2] = $20;
  $29 = 13832;
  $30 = $29;
  HEAP32[$30>>2] = $17;
  $31 = (($29) + 4)|0;
  $32 = $31;
  HEAP32[$32>>2] = $20;
  $33 = ((($0)) + 8|0);
  $34 = HEAP32[$33>>2]|0;
  HEAP32[7] = $34;
  HEAPF32[3460] = 0.0;
  HEAPF32[(13844)>>2] = 0.0;
  STACKTOP = sp;return;
 }
 switch ($4|0) {
 case 0:  {
  $35 = HEAP32[3522]|0;
  $36 = ($35|0)==(8);
  if ($36) {
   $37 = ((($0)) + 24|0);
   $38 = $37;
   $39 = $38;
   $40 = HEAP32[$39>>2]|0;
   $41 = (($38) + 4)|0;
   $42 = $41;
   $43 = HEAP32[$42>>2]|0;
   $44 = 13832;
   $45 = $44;
   HEAP32[$45>>2] = $40;
   $46 = (($44) + 4)|0;
   $47 = $46;
   HEAP32[$47>>2] = $43;
  }
  ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13816>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13816+4>>2]|0;
  ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13832>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13832+4>>2]|0;
  $48 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
  $49 = $48 / 0.0;
  HEAPF32[3525] = $49;
  HEAP32[3526] = 0;
  $50 = $49 > 5.0000002374872565E-4;
  if ($50) {
   $51 = HEAP32[7]|0;
   $52 = ((($0)) + 8|0);
   $53 = HEAP32[$52>>2]|0;
   $54 = ($51|0)==($53|0);
   if ($54) {
    ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13816>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13816+4>>2]|0;
    ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13832>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13832+4>>2]|0;
    $55 = (+_Vector2Angle($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
    $56 = 360.0 - $55;
    HEAPF32[3527] = $56;
    $57 = $56 < 30.0;
    $58 = $56 > 330.0;
    $or$cond3 = $57 | $58;
    if ($or$cond3) {
     $$sink10 = 16;
    } else {
     $59 = $56 > 30.0;
     $60 = $56 < 120.0;
     $or$cond5 = $59 & $60;
     if ($or$cond5) {
      $$sink10 = 64;
     } else {
      $61 = $56 > 120.0;
      $62 = $56 < 210.0;
      $or$cond7 = $61 & $62;
      $63 = $56 > 210.0;
      $64 = $56 < 300.0;
      $or$cond9 = $63 & $64;
      $$sink = $or$cond9 ? 128 : 0;
      $$$sink = $or$cond7 ? 32 : $$sink;
      $$sink10 = $$$sink;
     }
    }
   } else {
    label = 16;
   }
  } else {
   label = 16;
  }
  if ((label|0) == 16) {
   HEAPF32[3525] = 0.0;
   HEAPF32[3527] = 0.0;
   $$sink10 = 0;
  }
  HEAP32[3522] = $$sink10;
  HEAPF32[3456] = 0.0;
  HEAPF32[(13828)>>2] = 0.0;
  HEAP32[3523] = 0;
  STACKTOP = sp;return;
  break;
 }
 case 2:  {
  $65 = HEAP32[3526]|0;
  $66 = ($65|0)==(0);
  if ($66) {
   HEAP32[3526] = 1;
  }
  $67 = ((($0)) + 24|0);
  $68 = $67;
  $69 = $68;
  $70 = HEAP32[$69>>2]|0;
  $71 = (($68) + 4)|0;
  $72 = $71;
  $73 = HEAP32[$72>>2]|0;
  $74 = 13848;
  $75 = $74;
  HEAP32[$75>>2] = $70;
  $76 = (($74) + 4)|0;
  $77 = $76;
  HEAP32[$77>>2] = $73;
  $78 = HEAP32[3522]|0;
  $79 = ($78|0)==(4);
  if ($79) {
   ;HEAP32[$moveDownPosition$byval_copy11>>2]=HEAP32[13816>>2]|0;HEAP32[$moveDownPosition$byval_copy11+4>>2]=HEAP32[13816+4>>2]|0;
   ;HEAP32[$moveDownPosition2$byval_copy12>>2]=HEAP32[13848>>2]|0;HEAP32[$moveDownPosition2$byval_copy12+4>>2]=HEAP32[13848+4>>2]|0;
   $80 = (+_Vector2Distance($moveDownPosition$byval_copy11,$moveDownPosition2$byval_copy12));
   $81 = !($80 >= 0.014999999664723873);
   if (!($81)) {
    HEAP32[3522] = 8;
   }
  }
  $82 = +HEAPF32[3462];
  $83 = +HEAPF32[3456];
  $84 = $82 - $83;
  HEAPF32[3460] = $84;
  $85 = +HEAPF32[(13852)>>2];
  $86 = +HEAPF32[(13828)>>2];
  $87 = $85 - $86;
  HEAPF32[(13844)>>2] = $87;
  STACKTOP = sp;return;
  break;
 }
 default: {
  STACKTOP = sp;return;
 }
 }
}
function _UpdateGestures() {
 var $$off = 0, $0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $or$cond3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3522]|0;
 $$off = (($0) + -1)|0;
 $1 = ($$off>>>0)<(2);
 $2 = HEAP32[3523]|0;
 $3 = ($2|0)<(2);
 $or$cond3 = $1 & $3;
 if ($or$cond3) {
  HEAP32[3522] = 4;
 }
 $4 = HEAP32[3522]|0;
 $5 = (($4) + -16)|0;
 $6 = $5 >>> 4;
 $7 = $5 << 28;
 $8 = $6 | $7;
 switch ($8|0) {
 case 0: case 1: case 3: case 7:  {
  break;
 }
 default: {
  return;
 }
 }
 HEAP32[3522] = 0;
 return;
}
function _GetMousePosition($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = 13880;
 $2 = $1;
 $3 = HEAP32[$2>>2]|0;
 $4 = (($1) + 4)|0;
 $5 = $4;
 $6 = HEAP32[$5>>2]|0;
 $7 = $0;
 $8 = $7;
 HEAP32[$8>>2] = $3;
 $9 = (($7) + 4)|0;
 $10 = $9;
 HEAP32[$10>>2] = $6;
 return;
}
function _GetScreenWidth() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3532]|0;
 return ($0|0);
}
function _GetScreenHeight() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3531]|0;
 return ($0|0);
}
function _InitWindow($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $10 = 0, $3 = 0, $4 = 0.0, $5 = 0.0, $6 = 0, $7 = 0.0, $8 = 0.0, $9 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 _TraceLog(0,3595,$vararg_buffer);
 HEAP32[3534] = $2;
 _InitGraphicsDevice($0,$1);
 _LoadDefaultFont();
 _InitTimer();
 (_emscripten_set_fullscreenchange_callback((0|0),(0|0),1,(4|0))|0);
 (_emscripten_set_keypress_callback((3624|0),(0|0),1,(5|0))|0);
 (_emscripten_set_click_callback((3624|0),(0|0),1,(6|0))|0);
 (_emscripten_set_touchstart_callback((3624|0),(0|0),1,(7|0))|0);
 (_emscripten_set_touchend_callback((3624|0),(0|0),1,(7|0))|0);
 (_emscripten_set_touchmove_callback((3624|0),(0|0),1,(7|0))|0);
 (_emscripten_set_touchcancel_callback((3624|0),(0|0),1,(7|0))|0);
 (_emscripten_set_gamepadconnected_callback((0|0),1,(8|0))|0);
 (_emscripten_set_gamepaddisconnected_callback((0|0),1,(8|0))|0);
 $3 = HEAP32[3532]|0;
 $4 = (+($3|0));
 $5 = $4 * 0.5;
 HEAPF32[3470] = $5;
 $6 = HEAP32[3531]|0;
 $7 = (+($6|0));
 $8 = $7 * 0.5;
 HEAPF32[(13884)>>2] = $8;
 $9 = HEAP32[3535]|0;
 $10 = ($9|0)==(0);
 if ($10) {
  STACKTOP = sp;return;
 }
 _SetTargetFPS(60);
 _LogoAnimation();
 STACKTOP = sp;return;
}
function _TraceLog($0,$1,$varargs) {
 $0 = $0|0;
 $1 = $1|0;
 $varargs = $varargs|0;
 var $10 = 0, $11 = 0, $12 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $endptr = 0, $strlen = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $2 = sp;
 switch ($0|0) {
 case 0:  {
  ;HEAP8[13920>>0]=HEAP8[8153>>0]|0;HEAP8[13920+1>>0]=HEAP8[8153+1>>0]|0;HEAP8[13920+2>>0]=HEAP8[8153+2>>0]|0;HEAP8[13920+3>>0]=HEAP8[8153+3>>0]|0;HEAP8[13920+4>>0]=HEAP8[8153+4>>0]|0;HEAP8[13920+5>>0]=HEAP8[8153+5>>0]|0;HEAP8[13920+6>>0]=HEAP8[8153+6>>0]|0;
  break;
 }
 case 1:  {
  $3 = 13920;
  $4 = $3;
  HEAP32[$4>>2] = 1330795077;
  $5 = (($3) + 4)|0;
  $6 = $5;
  HEAP32[$6>>2] = 2112082;
  break;
 }
 case 2:  {
  dest=13920; src=8160; stop=dest+10|0; do { HEAP8[dest>>0]=HEAP8[src>>0]|0; dest=dest+1|0; src=src+1|0; } while ((dest|0) < (stop|0));
  break;
 }
 case 3:  {
  $7 = 13920;
  $8 = $7;
  HEAP32[$8>>2] = 1430406468;
  $9 = (($7) + 4)|0;
  $10 = $9;
  HEAP32[$10>>2] = 2112071;
  break;
 }
 default: {
 }
 }
 (_strcat(13920,$1)|0);
 $strlen = (_strlen(13920)|0);
 $endptr = (13920 + ($strlen)|0);
 HEAP8[$endptr>>0]=10&255;HEAP8[$endptr+1>>0]=10>>8;
 HEAP32[$2>>2] = $varargs;
 $11 = ($0|0)==(3);
 if ($11) {
  STACKTOP = sp;return;
 }
 (_vprintf(13920,$2)|0);
 $12 = ($0|0)==(1);
 if ($12) {
  _exit(1);
  // unreachable;
 } else {
  STACKTOP = sp;return;
 }
}
function _InitGraphicsDevice($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$015 = 0, $$byval_copy = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0;
 var $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0;
 var $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0;
 var $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0.0, $79 = 0, $8 = 0, $80 = 0;
 var $81 = 0, $82 = 0.0, $83 = 0, $84 = 0, $85 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer10 = 0, $vararg_buffer14 = 0, $vararg_buffer18 = 0, $vararg_buffer22 = 0, $vararg_buffer3 = 0, $vararg_buffer6 = 0, $vararg_buffer8 = 0, $vararg_ptr13 = 0, $vararg_ptr17 = 0, $vararg_ptr21 = 0, $vararg_ptr5 = 0, dest = 0;
 var label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 144|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(144|0);
 $$byval_copy = sp + 136|0;
 $vararg_buffer22 = sp + 64|0;
 $vararg_buffer18 = sp + 56|0;
 $vararg_buffer14 = sp + 48|0;
 $vararg_buffer10 = sp + 40|0;
 $vararg_buffer8 = sp + 32|0;
 $vararg_buffer6 = sp + 24|0;
 $vararg_buffer3 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $2 = sp + 72|0;
 $3 = sp + 140|0;
 HEAP32[3532] = $0;
 HEAP32[3531] = $1;
 _MatrixIdentity($2);
 dest=14216; src=$2; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 (_glfwSetErrorCallback((1|0))|0);
 $4 = (_glfwInit()|0);
 $5 = ($4|0)==(0);
 if ($5) {
  _TraceLog(1,4295,$vararg_buffer);
 }
 $6 = HEAP32[3532]|0;
 HEAP32[3570] = $6;
 $7 = HEAP32[3531]|0;
 HEAP32[3571] = $7;
 _glfwDefaultWindowHints();
 $8 = HEAP8[16932]|0;
 $9 = $8 & 4;
 $10 = ($9<<24>>24)==(0);
 if ($10) {
  _glfwWindowHint(131075,0);
 } else {
  _glfwWindowHint(131075,1);
 }
 $11 = HEAP8[16932]|0;
 $12 = $11 & 8;
 $13 = ($12<<24>>24)==(0);
 if (!($13)) {
  _glfwWindowHint(131077,1);
 }
 $14 = HEAP8[16932]|0;
 $15 = $14 & 32;
 $16 = ($15<<24>>24)==(0);
 if (!($16)) {
  _glfwWindowHint(135181,4);
  _TraceLog(0,4321,$vararg_buffer1);
 }
 $17 = (_rlGetVersion()|0);
 $18 = ($17|0)==(2);
 if ($18) {
  _glfwWindowHint(139266,2);
  _glfwWindowHint(139267,1);
 } else {
  $19 = (_rlGetVersion()|0);
  $20 = ($19|0)==(3);
  if ($20) {
   _glfwWindowHint(139266,3);
   _glfwWindowHint(139267,3);
   _glfwWindowHint(139272,204801);
   _glfwWindowHint(139270,0);
  }
 }
 $21 = HEAP32[3572]|0;
 $22 = ($21|0)==(0);
 if ($22) {
  $47 = HEAP32[3532]|0;
  $48 = HEAP32[3531]|0;
  $49 = HEAP32[3534]|0;
  $50 = (_glfwCreateWindow(($47|0),($48|0),($49|0),(0|0),(0|0))|0);
  HEAP32[3530] = $50;
  $51 = HEAP32[3532]|0;
  HEAP32[3573] = $51;
  $52 = HEAP32[3531]|0;
  HEAP32[3574] = $52;
  $54 = $50;
 } else {
  $23 = (_glfwGetPrimaryMonitor()|0);
  $24 = (_glfwGetVideoModes(($23|0),($$byval_copy|0))|0);
  $25 = HEAP32[$$byval_copy>>2]|0;
  $26 = ($25|0)>(0);
  L22: do {
   if ($26) {
    $27 = HEAP32[3532]|0;
    $28 = HEAP32[$$byval_copy>>2]|0;
    $29 = HEAP32[3531]|0;
    $$015 = 0;
    while(1) {
     $30 = (($24) + (($$015*24)|0)|0);
     $31 = HEAP32[$30>>2]|0;
     $32 = ($31|0)<($27|0);
     if (!($32)) {
      $33 = (((($24) + (($$015*24)|0)|0)) + 4|0);
      $34 = HEAP32[$33>>2]|0;
      $35 = ($34|0)<($29|0);
      if (!($35)) {
       break;
      }
     }
     $36 = (($$015) + 1)|0;
     $37 = ($36|0)<($28|0);
     if ($37) {
      $$015 = $36;
     } else {
      break L22;
     }
    }
    HEAP32[3570] = $31;
    HEAP32[3571] = $34;
   }
  } while(0);
  $38 = HEAP32[3570]|0;
  $39 = HEAP32[3571]|0;
  HEAP32[$vararg_buffer3>>2] = $38;
  $vararg_ptr5 = ((($vararg_buffer3)) + 4|0);
  HEAP32[$vararg_ptr5>>2] = $39;
  _TraceLog(2,4346,$vararg_buffer3);
  $40 = HEAP32[3570]|0;
  $41 = HEAP32[3571]|0;
  _SetupFramebufferSize($40,$41);
  $42 = HEAP32[3570]|0;
  $43 = HEAP32[3571]|0;
  $44 = HEAP32[3534]|0;
  $45 = (_glfwGetPrimaryMonitor()|0);
  $46 = (_glfwCreateWindow(($42|0),($43|0),($44|0),($45|0),(0|0))|0);
  HEAP32[3530] = $46;
  $54 = $46;
 }
 $53 = ($54|0)==(0|0);
 if ($53) {
  _glfwTerminate();
  _TraceLog(1,4384,$vararg_buffer6);
 } else {
  _TraceLog(0,4417,$vararg_buffer8);
  $55 = HEAP32[3573]|0;
  $56 = HEAP32[3574]|0;
  HEAP32[$vararg_buffer10>>2] = $55;
  $vararg_ptr13 = ((($vararg_buffer10)) + 4|0);
  HEAP32[$vararg_ptr13>>2] = $56;
  _TraceLog(0,4457,$vararg_buffer10);
  $57 = HEAP32[3532]|0;
  $58 = HEAP32[3531]|0;
  HEAP32[$vararg_buffer14>>2] = $57;
  $vararg_ptr17 = ((($vararg_buffer14)) + 4|0);
  HEAP32[$vararg_ptr17>>2] = $58;
  _TraceLog(0,4478,$vararg_buffer14);
  $59 = HEAP32[3575]|0;
  $60 = HEAP32[3576]|0;
  HEAP32[$vararg_buffer18>>2] = $59;
  $vararg_ptr21 = ((($vararg_buffer18)) + 4|0);
  HEAP32[$vararg_ptr21>>2] = $60;
  _TraceLog(0,4499,$vararg_buffer18);
 }
 $61 = HEAP32[3530]|0;
 (_glfwSetWindowSizeCallback(($61|0),(1|0))|0);
 $62 = HEAP32[3530]|0;
 (_glfwSetCursorEnterCallback(($62|0),(2|0))|0);
 $63 = HEAP32[3530]|0;
 (_glfwSetKeyCallback(($63|0),(1|0))|0);
 $64 = HEAP32[3530]|0;
 (_glfwSetMouseButtonCallback(($64|0),(1|0))|0);
 $65 = HEAP32[3530]|0;
 (_glfwSetCursorPosCallback(($65|0),(1|0))|0);
 $66 = HEAP32[3530]|0;
 (_glfwSetCharCallback(($66|0),(3|0))|0);
 $67 = HEAP32[3530]|0;
 (_glfwSetScrollCallback(($67|0),(2|0))|0);
 $68 = HEAP32[3530]|0;
 (_glfwSetWindowIconifyCallback(($68|0),(4|0))|0);
 $69 = HEAP32[3530]|0;
 _glfwMakeContextCurrent(($69|0));
 _glfwSwapInterval(0);
 $70 = HEAP8[16932]|0;
 $71 = $70 & 64;
 $72 = ($71<<24>>24)==(0);
 if ($72) {
  $73 = HEAP32[3532]|0;
  $74 = HEAP32[3531]|0;
  _rlglInit($73,$74);
  _SetupViewport();
  _rlMatrixMode(5889);
  _rlLoadIdentity();
  $75 = HEAP32[3573]|0;
  $76 = HEAP32[3575]|0;
  $77 = (($75) - ($76))|0;
  $78 = (+($77|0));
  $79 = HEAP32[3574]|0;
  $80 = HEAP32[3576]|0;
  $81 = (($79) - ($80))|0;
  $82 = (+($81|0));
  _rlOrtho(0.0,$78,$82,0.0,0.0,1.0);
  _rlMatrixMode(5888);
  _rlLoadIdentity();
  HEAP8[$3>>0] = -11;
  $83 = ((($3)) + 1|0);
  HEAP8[$83>>0] = -11;
  $84 = ((($3)) + 2|0);
  HEAP8[$84>>0] = -11;
  $85 = ((($3)) + 3|0);
  HEAP8[$85>>0] = -1;
  ;HEAP8[$$byval_copy>>0]=HEAP8[$3>>0]|0;HEAP8[$$byval_copy+1>>0]=HEAP8[$3+1>>0]|0;HEAP8[$$byval_copy+2>>0]=HEAP8[$3+2>>0]|0;HEAP8[$$byval_copy+3>>0]=HEAP8[$3+3>>0]|0;
  _ClearBackground($$byval_copy);
  STACKTOP = sp;return;
 }
 _glfwSwapInterval(1);
 _TraceLog(0,4524,$vararg_buffer22);
 $73 = HEAP32[3532]|0;
 $74 = HEAP32[3531]|0;
 _rlglInit($73,$74);
 _SetupViewport();
 _rlMatrixMode(5889);
 _rlLoadIdentity();
 $75 = HEAP32[3573]|0;
 $76 = HEAP32[3575]|0;
 $77 = (($75) - ($76))|0;
 $78 = (+($77|0));
 $79 = HEAP32[3574]|0;
 $80 = HEAP32[3576]|0;
 $81 = (($79) - ($80))|0;
 $82 = (+($81|0));
 _rlOrtho(0.0,$78,$82,0.0,0.0,1.0);
 _rlMatrixMode(5888);
 _rlLoadIdentity();
 HEAP8[$3>>0] = -11;
 $83 = ((($3)) + 1|0);
 HEAP8[$83>>0] = -11;
 $84 = ((($3)) + 2|0);
 HEAP8[$84>>0] = -11;
 $85 = ((($3)) + 3|0);
 HEAP8[$85>>0] = -1;
 ;HEAP8[$$byval_copy>>0]=HEAP8[$3>>0]|0;HEAP8[$$byval_copy+1>>0]=HEAP8[$3+1>>0]|0;HEAP8[$$byval_copy+2>>0]=HEAP8[$3+2>>0]|0;HEAP8[$$byval_copy+3>>0]=HEAP8[$3+3>>0]|0;
 _ClearBackground($$byval_copy);
 STACKTOP = sp;return;
}
function _LoadDefaultFont() {
 var $$ = 0, $$0101 = 0, $$090100 = 0, $$09299 = 0, $$095104 = 0, $$096103 = 0, $$097102 = 0, $$191 = 0, $$193 = 0, $$byval_copy1 = 0, $$lcssa = 0, $$sroa$0$0$$sroa_idx = 0, $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0;
 var $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0;
 var $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0;
 var $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $$byval_copy1 = sp + 44|0;
 $vararg_buffer = sp;
 $0 = sp + 4|0;
 $1 = sp + 24|0;
 HEAP32[(14184)>>2] = 224;
 $2 = (_malloc(65536)|0);
 _memset(($2|0),0,65536)|0;
 $$095104 = 0;$$096103 = 0;
 while(1) {
  $3 = (32 + ($$095104<<2)|0);
  $4 = HEAP32[$3>>2]|0;
  $$097102 = 31;
  while(1) {
   $16 = 1 << $$097102;
   $17 = $4 & $16;
   $18 = ($17|0)==(0);
   if (!($18)) {
    $19 = (($$097102) + ($$096103))|0;
    $$sroa$0$0$$sroa_idx = (($2) + ($19<<2)|0);
    HEAP8[$$sroa$0$0$$sroa_idx>>0]=-1&255;HEAP8[$$sroa$0$0$$sroa_idx+1>>0]=(-1>>8)&255;HEAP8[$$sroa$0$0$$sroa_idx+2>>0]=(-1>>16)&255;HEAP8[$$sroa$0$0$$sroa_idx+3>>0]=-1>>24;
   }
   $20 = (($$097102) + -1)|0;
   $21 = ($$097102|0)>(0);
   if ($21) {
    $$097102 = $20;
   } else {
    break;
   }
  }
  $12 = (($$095104) + 1)|0;
  $13 = ($$095104|0)>(511);
  $$ = $13 ? 0 : $12;
  $14 = (($$096103) + 32)|0;
  $15 = ($14|0)<(16384);
  if ($15) {
   $$095104 = $$;$$096103 = $14;
  } else {
   break;
  }
 }
 _LoadImageEx($0,$2,128,128);
 _ImageFormat($0,2);
 _free($2);
 ;HEAP32[$$byval_copy1>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy1+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy1+8>>2]=HEAP32[$0+8>>2]|0;HEAP32[$$byval_copy1+12>>2]=HEAP32[$0+12>>2]|0;HEAP32[$$byval_copy1+16>>2]=HEAP32[$0+16>>2]|0;
 _LoadTextureFromImage($1,$$byval_copy1);
 ;HEAP32[14160>>2]=HEAP32[$1>>2]|0;HEAP32[14160+4>>2]=HEAP32[$1+4>>2]|0;HEAP32[14160+8>>2]=HEAP32[$1+8>>2]|0;HEAP32[14160+12>>2]=HEAP32[$1+12>>2]|0;HEAP32[14160+16>>2]=HEAP32[$1+16>>2]|0;
 ;HEAP32[$$byval_copy1>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy1+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy1+8>>2]=HEAP32[$0+8>>2]|0;HEAP32[$$byval_copy1+12>>2]=HEAP32[$0+12>>2]|0;HEAP32[$$byval_copy1+16>>2]=HEAP32[$0+16>>2]|0;
 _UnloadImage($$byval_copy1);
 $5 = HEAP32[(14184)>>2]|0;
 $6 = $5 << 5;
 $7 = (_malloc($6)|0);
 HEAP32[(14188)>>2] = $7;
 $8 = ($5|0)>(0);
 if (!($8)) {
  $$lcssa = $7;
  $22 = ((($$lcssa)) + 16|0);
  $23 = HEAP32[$22>>2]|0;
  HEAP32[(14180)>>2] = $23;
  $24 = HEAP32[3540]|0;
  HEAP32[$vararg_buffer>>2] = $24;
  _TraceLog(0,3819,$vararg_buffer);
  STACKTOP = sp;return;
 }
 $9 = HEAP32[(14164)>>2]|0;
 $10 = HEAP32[(14184)>>2]|0;
 $11 = HEAP32[(14188)>>2]|0;
 $$0101 = 0;$$090100 = 1;$$09299 = 0;$27 = $7;
 while(1) {
  $25 = (($$0101) + 32)|0;
  $26 = (($27) + ($$0101<<5)|0);
  HEAP32[$26>>2] = $25;
  $28 = (((($27) + ($$0101<<5)|0)) + 4|0);
  HEAP32[$28>>2] = $$090100;
  $29 = ($$09299*11)|0;
  $30 = (($29) + 1)|0;
  $31 = (((($27) + ($$0101<<5)|0)) + 8|0);
  HEAP32[$31>>2] = $30;
  $32 = (2080 + ($$0101<<2)|0);
  $33 = HEAP32[$32>>2]|0;
  $34 = (((($27) + ($$0101<<5)|0)) + 12|0);
  HEAP32[$34>>2] = $33;
  $35 = (((($27) + ($$0101<<5)|0)) + 16|0);
  HEAP32[$35>>2] = 10;
  $36 = (($$090100) + 1)|0;
  $37 = (($36) + ($33))|0;
  $38 = ($37|0)<($9|0);
  $39 = (($$09299) + 1)|0;
  if ($38) {
   $$191 = $37;$$193 = $$09299;
  } else {
   $40 = ($39*11)|0;
   $41 = (($40) + 1)|0;
   $42 = (($33) + 2)|0;
   HEAP32[$28>>2] = 1;
   HEAP32[$31>>2] = $41;
   $$191 = $42;$$193 = $39;
  }
  $43 = (((($27) + ($$0101<<5)|0)) + 20|0);
  HEAP32[$43>>2] = 0;
  $44 = (((($27) + ($$0101<<5)|0)) + 24|0);
  HEAP32[$44>>2] = 0;
  $45 = (((($27) + ($$0101<<5)|0)) + 28|0);
  HEAP32[$45>>2] = 0;
  $46 = (($$0101) + 1)|0;
  $47 = ($46|0)<($10|0);
  if ($47) {
   $$0101 = $46;$$090100 = $$191;$$09299 = $$193;$27 = $11;
  } else {
   $$lcssa = $11;
   break;
  }
 }
 $22 = ((($$lcssa)) + 16|0);
 $23 = HEAP32[$22>>2]|0;
 HEAP32[(14180)>>2] = $23;
 $24 = HEAP32[3540]|0;
 HEAP32[$vararg_buffer>>2] = $24;
 _TraceLog(0,3819,$vararg_buffer);
 STACKTOP = sp;return;
}
function _InitTimer() {
 var $0 = 0, $1 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (_time((0|0))|0);
 _srand($0);
 $1 = (+_GetTime());
 HEAPF64[1739] = $1;
 return;
}
function _EmscriptenFullscreenChangeCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $10 = 0, $11 = 0, $12 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer4 = 0, $vararg_ptr1 = 0, $vararg_ptr2 = 0, $vararg_ptr3 = 0, $vararg_ptr7 = 0, $vararg_ptr8 = 0, $vararg_ptr9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $vararg_buffer4 = sp + 16|0;
 $vararg_buffer = sp;
 $3 = HEAP32[$1>>2]|0;
 $4 = ($3|0)==(0);
 $5 = ((($1)) + 264|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = ((($1)) + 268|0);
 $8 = HEAP32[$7>>2]|0;
 $9 = ((($1)) + 272|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = ((($1)) + 276|0);
 $12 = HEAP32[$11>>2]|0;
 if ($4) {
  HEAP32[$vararg_buffer4>>2] = $6;
  $vararg_ptr7 = ((($vararg_buffer4)) + 4|0);
  HEAP32[$vararg_ptr7>>2] = $8;
  $vararg_ptr8 = ((($vararg_buffer4)) + 8|0);
  HEAP32[$vararg_ptr8>>2] = $10;
  $vararg_ptr9 = ((($vararg_buffer4)) + 12|0);
  HEAP32[$vararg_ptr9>>2] = $12;
  _TraceLog(0,3752,$vararg_buffer4);
  STACKTOP = sp;return 0;
 } else {
  HEAP32[$vararg_buffer>>2] = $6;
  $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
  HEAP32[$vararg_ptr1>>2] = $8;
  $vararg_ptr2 = ((($vararg_buffer)) + 8|0);
  HEAP32[$vararg_ptr2>>2] = $10;
  $vararg_ptr3 = ((($vararg_buffer)) + 12|0);
  HEAP32[$vararg_ptr3>>2] = $12;
  _TraceLog(0,3683,$vararg_buffer);
  STACKTOP = sp;return 0;
 }
 return (0)|0;
}
function _EmscriptenKeyboardCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $3 = 0, $4 = 0, $5 = 0, $6 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($0|0)==(1);
 if (!($3)) {
  return 0;
 }
 $4 = ((($1)) + 32|0);
 $5 = (_strcmp($4,3676)|0);
 $6 = ($5|0)==(0);
 if (!($6)) {
  return 0;
 }
 (_emscripten_exit_pointerlock()|0);
 return 0;
}
function _EmscriptenMouseCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $3 = 0, $4 = 0, $5 = 0, $6 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 272|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(272|0);
 $3 = sp;
 $4 = ($0|0)==(4);
 if (!($4)) {
  STACKTOP = sp;return 0;
 }
 (_emscripten_get_pointerlock_status(($3|0))|0);
 $5 = HEAP32[$3>>2]|0;
 $6 = ($5|0)==(0);
 if ($6) {
  (_emscripten_request_pointerlock((0|0),1)|0);
 } else {
  (_emscripten_exit_pointerlock()|0);
  (_emscripten_get_pointerlock_status(($3|0))|0);
 }
 STACKTOP = sp;return 0;
}
function _EmscriptenTouchCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$byval_copy = 0, $$sink = 0, $$sroa$0$0$$sroa_idx = 0, $$sroa$03$0$$sroa_idx = 0, $$sroa$2$0$$sroa_idx2 = 0, $$sroa$24$0$$sroa_idx5 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0.0, $15 = 0, $16 = 0, $17 = 0.0, $18 = 0, $19 = 0, $20 = 0.0, $21 = 0, $22 = 0, $23 = 0.0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0;
 var $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0.0, $48 = 0.0, $49 = 0.0, $5 = 0, $50 = 0, $51 = 0.0, $52 = 0.0, $53 = 0.0, $54 = 0, $55 = 0.0, $56 = 0.0, $57 = 0.0, $58 = 0, $59 = 0.0, $6 = 0;
 var $60 = 0.0, $61 = 0.0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 112|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(112|0);
 $$byval_copy = sp + 56|0;
 $3 = sp;
 switch ($0|0) {
 case 22:  {
  $$sink = 1;
  label = 4;
  break;
 }
 case 23:  {
  $$sink = 0;
  label = 4;
  break;
 }
 case 24:  {
  $$sink = 2;
  label = 4;
  break;
 }
 default: {
 }
 }
 if ((label|0) == 4) {
  HEAP32[$3>>2] = $$sink;
 }
 $4 = HEAP32[$1>>2]|0;
 $5 = ((($3)) + 4|0);
 HEAP32[$5>>2] = $4;
 $6 = ((($1)) + 20|0);
 $7 = HEAP32[$6>>2]|0;
 $8 = ((($3)) + 8|0);
 HEAP32[$8>>2] = $7;
 $9 = ((($1)) + 72|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = ((($3)) + 12|0);
 HEAP32[$11>>2] = $10;
 $12 = ((($1)) + 56|0);
 $13 = HEAP32[$12>>2]|0;
 $14 = (+($13|0));
 $15 = ((($1)) + 60|0);
 $16 = HEAP32[$15>>2]|0;
 $17 = (+($16|0));
 $$sroa$03$0$$sroa_idx = ((($3)) + 24|0);
 HEAPF32[$$sroa$03$0$$sroa_idx>>2] = $14;
 $$sroa$24$0$$sroa_idx5 = ((($3)) + 28|0);
 HEAPF32[$$sroa$24$0$$sroa_idx5>>2] = $17;
 $18 = ((($1)) + 108|0);
 $19 = HEAP32[$18>>2]|0;
 $20 = (+($19|0));
 $21 = ((($1)) + 112|0);
 $22 = HEAP32[$21>>2]|0;
 $23 = (+($22|0));
 $$sroa$0$0$$sroa_idx = ((($3)) + 32|0);
 HEAPF32[$$sroa$0$0$$sroa_idx>>2] = $20;
 $$sroa$2$0$$sroa_idx2 = ((($3)) + 36|0);
 HEAPF32[$$sroa$2$0$$sroa_idx2>>2] = $23;
 $24 = ((($3)) + 24|0);
 $25 = $24;
 $26 = $25;
 $27 = HEAP32[$26>>2]|0;
 $28 = (($25) + 4)|0;
 $29 = $28;
 $30 = HEAP32[$29>>2]|0;
 $31 = 13896;
 $32 = $31;
 HEAP32[$32>>2] = $27;
 $33 = (($31) + 4)|0;
 $34 = $33;
 HEAP32[$34>>2] = $30;
 $35 = ((($3)) + 32|0);
 $36 = $35;
 $37 = $36;
 $38 = HEAP32[$37>>2]|0;
 $39 = (($36) + 4)|0;
 $40 = $39;
 $41 = HEAP32[$40>>2]|0;
 $42 = (13904);
 $43 = $42;
 HEAP32[$43>>2] = $38;
 $44 = (($42) + 4)|0;
 $45 = $44;
 HEAP32[$45>>2] = $41;
 $46 = (_GetScreenWidth()|0);
 $47 = (+($46|0));
 $48 = +HEAPF32[$24>>2];
 $49 = $48 / $47;
 HEAPF32[$24>>2] = $49;
 $50 = (_GetScreenHeight()|0);
 $51 = (+($50|0));
 $52 = +HEAPF32[$$sroa$24$0$$sroa_idx5>>2];
 $53 = $52 / $51;
 HEAPF32[$$sroa$24$0$$sroa_idx5>>2] = $53;
 $54 = (_GetScreenWidth()|0);
 $55 = (+($54|0));
 $56 = +HEAPF32[$35>>2];
 $57 = $56 / $55;
 HEAPF32[$35>>2] = $57;
 $58 = (_GetScreenHeight()|0);
 $59 = (+($58|0));
 $60 = +HEAPF32[$$sroa$2$0$$sroa_idx2>>2];
 $61 = $60 / $59;
 HEAPF32[$$sroa$2$0$$sroa_idx2>>2] = $61;
 dest=$$byval_copy; src=$3; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _ProcessGestureEvent($$byval_copy);
 STACKTOP = sp;return 1;
}
function _EmscriptenGamepadCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$sink = 0, $10 = 0, $11 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ((($1)) + 1296|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ($4|0)==(0);
 if ($5) {
  label = 3;
 } else {
  $6 = ((($1)) + 1300|0);
  $7 = HEAP32[$6>>2]|0;
  $8 = ($7|0)<(4);
  if ($8) {
   $$sink = 1;
  } else {
   label = 3;
  }
 }
 if ((label|0) == 3) {
  $$sink = 0;
 }
 $9 = ((($1)) + 1300|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = (14144 + ($10<<2)|0);
 HEAP32[$11>>2] = $$sink;
 return 0;
}
function _SetTargetFPS($0) {
 $0 = $0|0;
 var $$ = 0.0, $$op = 0.0, $1 = 0, $2 = 0.0, $3 = 0.0, $4 = 0.0, $5 = 0.0, $6 = 0.0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 $1 = ($0|0)<(1);
 $2 = (+($0|0));
 $3 = 1.0 / $2;
 $$ = $1 ? 0.0 : $3;
 HEAPF64[1736] = $$;
 $4 = $3;
 $$op = $4 * 1000.0;
 $5 = $$op;
 $6 = $1 ? 0.0 : $5;
 HEAPF64[$vararg_buffer>>3] = $6;
 _TraceLog(0,3632,$vararg_buffer);
 STACKTOP = sp;return;
}
function _LogoAnimation() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 HEAP32[3535] = 0;
 return;
}
function _GetTime() {
 var $0 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (+_glfwGetTime());
 return (+$0);
}
function _LoadImageEx($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $$03334 = 0, $$035 = 0, $$sroa$12$0$$sroa_idx21 = 0, $$sroa$15$0$$sroa_idx24 = 0, $$sroa$16$0$$sroa_idx26 = 0, $$sroa$9$0$$sroa_idx18 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $exitcond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $4 = $2 << 2;
 $5 = Math_imul($4, $3)|0;
 $6 = (_malloc($5)|0);
 $7 = ($5|0)>(0);
 if ($7) {
  $8 = (($5) + -1)|0;
  $9 = $8 >>> 2;
  $$03334 = 0;$$035 = 0;
  while(1) {
   $10 = (($1) + ($$03334<<2)|0);
   $11 = HEAP8[$10>>0]|0;
   $12 = (($6) + ($$035)|0);
   HEAP8[$12>>0] = $11;
   $13 = (((($1) + ($$03334<<2)|0)) + 1|0);
   $14 = HEAP8[$13>>0]|0;
   $15 = $$035 | 1;
   $16 = (($6) + ($15)|0);
   HEAP8[$16>>0] = $14;
   $17 = (((($1) + ($$03334<<2)|0)) + 2|0);
   $18 = HEAP8[$17>>0]|0;
   $19 = $$035 | 2;
   $20 = (($6) + ($19)|0);
   HEAP8[$20>>0] = $18;
   $21 = (((($1) + ($$03334<<2)|0)) + 3|0);
   $22 = HEAP8[$21>>0]|0;
   $23 = $$035 | 3;
   $24 = (($6) + ($23)|0);
   HEAP8[$24>>0] = $22;
   $25 = (($$03334) + 1)|0;
   $26 = (($$035) + 4)|0;
   $exitcond = ($$03334|0)==($9|0);
   if ($exitcond) {
    break;
   } else {
    $$03334 = $25;$$035 = $26;
   }
  }
 }
 HEAP32[$0>>2] = $6;
 $$sroa$9$0$$sroa_idx18 = ((($0)) + 4|0);
 HEAP32[$$sroa$9$0$$sroa_idx18>>2] = $2;
 $$sroa$12$0$$sroa_idx21 = ((($0)) + 8|0);
 HEAP32[$$sroa$12$0$$sroa_idx21>>2] = $3;
 $$sroa$15$0$$sroa_idx24 = ((($0)) + 12|0);
 HEAP32[$$sroa$15$0$$sroa_idx24>>2] = 1;
 $$sroa$16$0$$sroa_idx26 = ((($0)) + 16|0);
 HEAP32[$$sroa$16$0$$sroa_idx26>>2] = 7;
 return;
}
function _ImageFormat($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0166199 = 0, $$0167197 = 0, $$0168195 = 0, $$0169192 = 0, $$0170190 = 0, $$0171188 = 0, $$0172189 = 0, $$0202 = 0, $$1194 = 0, $$2201 = 0, $$byval_copy = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0.0, $103 = 0.0, $104 = 0.0, $105 = 0, $106 = 0, $107 = 0;
 var $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0;
 var $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0;
 var $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0, $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0, $16 = 0, $160 = 0, $161 = 0;
 var $162 = 0, $163 = 0, $164 = 0, $165 = 0, $166 = 0, $167 = 0, $168 = 0, $169 = 0.0, $17 = 0, $170 = 0.0, $171 = 0.0, $172 = 0, $173 = 0, $174 = 0, $175 = 0.0, $176 = 0.0, $177 = 0.0, $178 = 0, $179 = 0, $18 = 0;
 var $180 = 0, $181 = 0.0, $182 = 0.0, $183 = 0.0, $184 = 0, $185 = 0, $186 = 0, $187 = 0, $188 = 0, $189 = 0, $19 = 0.0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0, $197 = 0, $198 = 0;
 var $199 = 0, $2 = 0, $20 = 0.0, $200 = 0, $201 = 0, $202 = 0, $203 = 0, $204 = 0, $205 = 0, $206 = 0, $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0, $212 = 0, $213 = 0, $214 = 0, $215 = 0;
 var $216 = 0, $217 = 0, $218 = 0.0, $219 = 0.0, $22 = 0, $220 = 0.0, $221 = 0, $222 = 0, $223 = 0, $224 = 0.0, $225 = 0.0, $226 = 0.0, $227 = 0, $228 = 0, $229 = 0, $23 = 0.0, $230 = 0.0, $231 = 0.0, $232 = 0.0, $233 = 0;
 var $234 = 0, $235 = 0, $236 = 0.0, $237 = 0.0, $238 = 0.0, $239 = 0, $24 = 0.0, $240 = 0, $241 = 0, $242 = 0, $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0, $249 = 0, $25 = 0.0, $250 = 0, $251 = 0;
 var $252 = 0, $253 = 0, $254 = 0, $255 = 0, $256 = 0, $257 = 0, $258 = 0, $259 = 0, $26 = 0, $260 = 0, $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0, $267 = 0, $268 = 0, $269 = 0, $27 = 0;
 var $270 = 0, $271 = 0, $272 = 0, $273 = 0, $274 = 0, $275 = 0, $276 = 0, $277 = 0, $278 = 0, $279 = 0, $28 = 0.0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0, $285 = 0, $286 = 0, $287 = 0, $288 = 0;
 var $289 = 0, $29 = 0.0, $290 = 0, $3 = 0, $30 = 0.0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0;
 var $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0.0, $53 = 0.0, $54 = 0, $55 = 0, $56 = 0.0, $57 = 0.0, $58 = 0.0, $59 = 0, $6 = 0, $60 = 0, $61 = 0.0, $62 = 0.0;
 var $63 = 0.0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0;
 var $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0.0, $91 = 0.0, $92 = 0.0, $93 = 0, $94 = 0, $95 = 0, $96 = 0.0, $97 = 0.0, $98 = 0.0, $99 = 0;
 var $or$cond = 0, $roundf = 0.0, $roundf173 = 0.0, $roundf174 = 0.0, $roundf175 = 0.0, $roundf176 = 0.0, $roundf177 = 0.0, $roundf178 = 0.0, $roundf179 = 0.0, $roundf180 = 0.0, $roundf181 = 0.0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $$byval_copy = sp + 4|0;
 $vararg_buffer = sp;
 $2 = ((($0)) + 16|0);
 $3 = HEAP32[$2>>2]|0;
 $4 = ($3|0)==($1|0);
 if ($4) {
  STACKTOP = sp;return;
 }
 $5 = ($3|0)<(8);
 $6 = ($1|0)<(8);
 $or$cond = $6 & $5;
 if (!($or$cond)) {
  _TraceLog(2,4195,$vararg_buffer);
  STACKTOP = sp;return;
 }
 ;HEAP32[$$byval_copy>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy+8>>2]=HEAP32[$0+8>>2]|0;HEAP32[$$byval_copy+12>>2]=HEAP32[$0+12>>2]|0;HEAP32[$$byval_copy+16>>2]=HEAP32[$0+16>>2]|0;
 $7 = (_GetImageData($$byval_copy)|0);
 $8 = HEAP32[$0>>2]|0;
 _free($8);
 HEAP32[$2>>2] = $1;
 switch ($1|0) {
 case 1:  {
  $9 = ((($0)) + 4|0);
  $10 = HEAP32[$9>>2]|0;
  $11 = ((($0)) + 8|0);
  $12 = HEAP32[$11>>2]|0;
  $13 = Math_imul($12, $10)|0;
  $14 = (_malloc($13)|0);
  HEAP32[$0>>2] = $14;
  $15 = Math_imul($12, $10)|0;
  $16 = ($15|0)>(0);
  if ($16) {
   $$0171188 = 0;
   while(1) {
    $17 = (($7) + ($$0171188<<2)|0);
    $18 = HEAP8[$17>>0]|0;
    $19 = (+($18&255));
    $20 = $19 * 0.29899999499320984;
    $21 = (((($7) + ($$0171188<<2)|0)) + 1|0);
    $22 = HEAP8[$21>>0]|0;
    $23 = (+($22&255));
    $24 = $23 * 0.58700001239776611;
    $25 = $20 + $24;
    $26 = (((($7) + ($$0171188<<2)|0)) + 2|0);
    $27 = HEAP8[$26>>0]|0;
    $28 = (+($27&255));
    $29 = $28 * 0.11400000005960464;
    $30 = $25 + $29;
    $31 = (~~(($30))&255);
    $32 = HEAP32[$0>>2]|0;
    $33 = (($32) + ($$0171188)|0);
    HEAP8[$33>>0] = $31;
    $34 = (($$0171188) + 1)|0;
    $35 = HEAP32[$9>>2]|0;
    $36 = HEAP32[$11>>2]|0;
    $37 = Math_imul($36, $35)|0;
    $38 = ($34|0)<($37|0);
    if ($38) {
     $$0171188 = $34;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 2:  {
  $39 = ((($0)) + 4|0);
  $40 = HEAP32[$39>>2]|0;
  $41 = ((($0)) + 8|0);
  $42 = HEAP32[$41>>2]|0;
  $43 = $40 << 1;
  $44 = Math_imul($43, $42)|0;
  $45 = (_malloc($44)|0);
  HEAP32[$0>>2] = $45;
  $46 = HEAP32[$39>>2]|0;
  $47 = $46 << 1;
  $48 = Math_imul($47, $42)|0;
  $49 = ($48|0)>(0);
  if ($49) {
   $$0170190 = 0;$$0172189 = 0;
   while(1) {
    $50 = (($7) + ($$0172189<<2)|0);
    $51 = HEAP8[$50>>0]|0;
    $52 = (+($51&255));
    $53 = $52 * 0.29899999499320984;
    $54 = (((($7) + ($$0172189<<2)|0)) + 1|0);
    $55 = HEAP8[$54>>0]|0;
    $56 = (+($55&255));
    $57 = $56 * 0.58700001239776611;
    $58 = $53 + $57;
    $59 = (((($7) + ($$0172189<<2)|0)) + 2|0);
    $60 = HEAP8[$59>>0]|0;
    $61 = (+($60&255));
    $62 = $61 * 0.11400000005960464;
    $63 = $58 + $62;
    $64 = (~~(($63))&255);
    $65 = HEAP32[$0>>2]|0;
    $66 = (($65) + ($$0170190)|0);
    HEAP8[$66>>0] = $64;
    $67 = (((($7) + ($$0172189<<2)|0)) + 3|0);
    $68 = HEAP8[$67>>0]|0;
    $69 = HEAP32[$0>>2]|0;
    $70 = $$0170190 | 1;
    $71 = (($69) + ($70)|0);
    HEAP8[$71>>0] = $68;
    $72 = (($$0172189) + 1)|0;
    $73 = (($$0170190) + 2)|0;
    $74 = HEAP32[$39>>2]|0;
    $75 = HEAP32[$41>>2]|0;
    $76 = $74 << 1;
    $77 = Math_imul($76, $75)|0;
    $78 = ($73|0)<($77|0);
    if ($78) {
     $$0170190 = $73;$$0172189 = $72;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 3:  {
  $79 = ((($0)) + 4|0);
  $80 = HEAP32[$79>>2]|0;
  $81 = ((($0)) + 8|0);
  $82 = HEAP32[$81>>2]|0;
  $83 = $80 << 1;
  $84 = Math_imul($83, $82)|0;
  $85 = (_malloc($84)|0);
  HEAP32[$0>>2] = $85;
  $86 = HEAP32[$79>>2]|0;
  $87 = Math_imul($82, $86)|0;
  $88 = ($87|0)>(0);
  if ($88) {
   $89 = HEAP8[$7>>0]|0;
   $90 = (+($89&255));
   $91 = $90 * 31.0;
   $92 = $91 / 255.0;
   $roundf179 = (+_roundf((+$92)));
   $93 = (~~(($roundf179))&255);
   $94 = ((($7)) + 1|0);
   $95 = HEAP8[$94>>0]|0;
   $96 = (+($95&255));
   $97 = $96 * 63.0;
   $98 = $97 / 255.0;
   $roundf180 = (+_roundf((+$98)));
   $99 = (~~(($roundf180))&255);
   $100 = ((($7)) + 2|0);
   $101 = HEAP8[$100>>0]|0;
   $102 = (+($101&255));
   $103 = $102 * 31.0;
   $104 = $103 / 255.0;
   $roundf181 = (+_roundf((+$104)));
   $105 = (~~(($roundf181))&255);
   $106 = $93&255;
   $107 = $106 << 11;
   $108 = $99&255;
   $109 = $108 << 5;
   $110 = $109 | $107;
   $111 = $105&255;
   $112 = $110 | $111;
   $113 = $112&65535;
   $114 = HEAP32[$0>>2]|0;
   $115 = HEAP32[$79>>2]|0;
   $116 = HEAP32[$81>>2]|0;
   $117 = Math_imul($116, $115)|0;
   $$0169192 = 0;
   while(1) {
    $118 = (($114) + ($$0169192<<1)|0);
    HEAP16[$118>>1] = $113;
    $119 = (($$0169192) + 1)|0;
    $120 = ($119|0)<($117|0);
    if ($120) {
     $$0169192 = $119;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 4:  {
  $121 = ((($0)) + 4|0);
  $122 = HEAP32[$121>>2]|0;
  $123 = ((($0)) + 8|0);
  $124 = HEAP32[$123>>2]|0;
  $125 = ($122*3)|0;
  $126 = Math_imul($125, $124)|0;
  $127 = (_malloc($126)|0);
  HEAP32[$0>>2] = $127;
  $128 = HEAP32[$121>>2]|0;
  $129 = ($128*3)|0;
  $130 = Math_imul($129, $124)|0;
  $131 = ($130|0)>(0);
  if ($131) {
   $$0168195 = 0;$$1194 = 0;
   while(1) {
    $132 = (($7) + ($$1194<<2)|0);
    $133 = HEAP8[$132>>0]|0;
    $134 = HEAP32[$0>>2]|0;
    $135 = (($134) + ($$0168195)|0);
    HEAP8[$135>>0] = $133;
    $136 = (((($7) + ($$1194<<2)|0)) + 1|0);
    $137 = HEAP8[$136>>0]|0;
    $138 = HEAP32[$0>>2]|0;
    $139 = (($$0168195) + 1)|0;
    $140 = (($138) + ($139)|0);
    HEAP8[$140>>0] = $137;
    $141 = (((($7) + ($$1194<<2)|0)) + 2|0);
    $142 = HEAP8[$141>>0]|0;
    $143 = HEAP32[$0>>2]|0;
    $144 = (($$0168195) + 2)|0;
    $145 = (($143) + ($144)|0);
    HEAP8[$145>>0] = $142;
    $146 = (($$1194) + 1)|0;
    $147 = (($$0168195) + 3)|0;
    $148 = HEAP32[$121>>2]|0;
    $149 = HEAP32[$123>>2]|0;
    $150 = ($148*3)|0;
    $151 = Math_imul($150, $149)|0;
    $152 = ($147|0)<($151|0);
    if ($152) {
     $$0168195 = $147;$$1194 = $146;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 5:  {
  $153 = ((($0)) + 4|0);
  $154 = HEAP32[$153>>2]|0;
  $155 = ((($0)) + 8|0);
  $156 = HEAP32[$155>>2]|0;
  $157 = $154 << 1;
  $158 = Math_imul($157, $156)|0;
  $159 = (_malloc($158)|0);
  HEAP32[$0>>2] = $159;
  $160 = HEAP32[$153>>2]|0;
  $161 = Math_imul($156, $160)|0;
  $162 = ($161|0)>(0);
  if ($162) {
   $163 = HEAP32[$0>>2]|0;
   $164 = HEAP32[$153>>2]|0;
   $165 = HEAP32[$155>>2]|0;
   $166 = Math_imul($165, $164)|0;
   $$0167197 = 0;
   while(1) {
    $167 = (($7) + ($$0167197<<2)|0);
    $168 = HEAP8[$167>>0]|0;
    $169 = (+($168&255));
    $170 = $169 * 31.0;
    $171 = $170 / 255.0;
    $roundf176 = (+_roundf((+$171)));
    $172 = (~~(($roundf176))&255);
    $173 = (((($7) + ($$0167197<<2)|0)) + 1|0);
    $174 = HEAP8[$173>>0]|0;
    $175 = (+($174&255));
    $176 = $175 * 31.0;
    $177 = $176 / 255.0;
    $roundf177 = (+_roundf((+$177)));
    $178 = (~~(($roundf177))&255);
    $179 = (((($7) + ($$0167197<<2)|0)) + 2|0);
    $180 = HEAP8[$179>>0]|0;
    $181 = (+($180&255));
    $182 = $181 * 31.0;
    $183 = $182 / 255.0;
    $roundf178 = (+_roundf((+$183)));
    $184 = (~~(($roundf178))&255);
    $185 = (((($7) + ($$0167197<<2)|0)) + 3|0);
    $186 = HEAP8[$185>>0]|0;
    $187 = ($186&255)>(50);
    $188 = $172&255;
    $189 = $188 << 11;
    $190 = $178&255;
    $191 = $190 << 6;
    $192 = $191 | $189;
    $193 = $184&255;
    $194 = $193 << 1;
    $195 = $192 | $194;
    $196 = $187&1;
    $197 = $195 | $196;
    $198 = $197&65535;
    $199 = (($163) + ($$0167197<<1)|0);
    HEAP16[$199>>1] = $198;
    $200 = (($$0167197) + 1)|0;
    $201 = ($200|0)<($166|0);
    if ($201) {
     $$0167197 = $200;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 6:  {
  $202 = ((($0)) + 4|0);
  $203 = HEAP32[$202>>2]|0;
  $204 = ((($0)) + 8|0);
  $205 = HEAP32[$204>>2]|0;
  $206 = $203 << 1;
  $207 = Math_imul($206, $205)|0;
  $208 = (_malloc($207)|0);
  HEAP32[$0>>2] = $208;
  $209 = HEAP32[$202>>2]|0;
  $210 = Math_imul($205, $209)|0;
  $211 = ($210|0)>(0);
  if ($211) {
   $212 = HEAP32[$0>>2]|0;
   $213 = HEAP32[$202>>2]|0;
   $214 = HEAP32[$204>>2]|0;
   $215 = Math_imul($214, $213)|0;
   $$0166199 = 0;
   while(1) {
    $216 = (($7) + ($$0166199<<2)|0);
    $217 = HEAP8[$216>>0]|0;
    $218 = (+($217&255));
    $219 = $218 * 15.0;
    $220 = $219 / 255.0;
    $roundf = (+_roundf((+$220)));
    $221 = (~~(($roundf))&255);
    $222 = (((($7) + ($$0166199<<2)|0)) + 1|0);
    $223 = HEAP8[$222>>0]|0;
    $224 = (+($223&255));
    $225 = $224 * 15.0;
    $226 = $225 / 255.0;
    $roundf173 = (+_roundf((+$226)));
    $227 = (~~(($roundf173))&255);
    $228 = (((($7) + ($$0166199<<2)|0)) + 2|0);
    $229 = HEAP8[$228>>0]|0;
    $230 = (+($229&255));
    $231 = $230 * 15.0;
    $232 = $231 / 255.0;
    $roundf174 = (+_roundf((+$232)));
    $233 = (~~(($roundf174))&255);
    $234 = (((($7) + ($$0166199<<2)|0)) + 3|0);
    $235 = HEAP8[$234>>0]|0;
    $236 = (+($235&255));
    $237 = $236 * 15.0;
    $238 = $237 / 255.0;
    $roundf175 = (+_roundf((+$238)));
    $239 = (~~(($roundf175))&255);
    $240 = $221&255;
    $241 = $240 << 12;
    $242 = $227&255;
    $243 = $242 << 8;
    $244 = $243 | $241;
    $245 = $233&255;
    $246 = $245 << 4;
    $247 = $244 | $246;
    $248 = $239&255;
    $249 = $247 | $248;
    $250 = $249&65535;
    $251 = (($212) + ($$0166199<<1)|0);
    HEAP16[$251>>1] = $250;
    $252 = (($$0166199) + 1)|0;
    $253 = ($252|0)<($215|0);
    if ($253) {
     $$0166199 = $252;
    } else {
     break;
    }
   }
  }
  break;
 }
 case 7:  {
  $254 = ((($0)) + 4|0);
  $255 = HEAP32[$254>>2]|0;
  $256 = ((($0)) + 8|0);
  $257 = HEAP32[$256>>2]|0;
  $258 = $255 << 2;
  $259 = Math_imul($258, $257)|0;
  $260 = (_malloc($259)|0);
  HEAP32[$0>>2] = $260;
  $261 = HEAP32[$254>>2]|0;
  $262 = $261 << 2;
  $263 = Math_imul($262, $257)|0;
  $264 = ($263|0)>(0);
  if ($264) {
   $$0202 = 0;$$2201 = 0;
   while(1) {
    $265 = (($7) + ($$2201<<2)|0);
    $266 = HEAP8[$265>>0]|0;
    $267 = HEAP32[$0>>2]|0;
    $268 = (($267) + ($$0202)|0);
    HEAP8[$268>>0] = $266;
    $269 = (((($7) + ($$2201<<2)|0)) + 1|0);
    $270 = HEAP8[$269>>0]|0;
    $271 = HEAP32[$0>>2]|0;
    $272 = $$0202 | 1;
    $273 = (($271) + ($272)|0);
    HEAP8[$273>>0] = $270;
    $274 = (((($7) + ($$2201<<2)|0)) + 2|0);
    $275 = HEAP8[$274>>0]|0;
    $276 = HEAP32[$0>>2]|0;
    $277 = $$0202 | 2;
    $278 = (($276) + ($277)|0);
    HEAP8[$278>>0] = $275;
    $279 = (((($7) + ($$2201<<2)|0)) + 3|0);
    $280 = HEAP8[$279>>0]|0;
    $281 = HEAP32[$0>>2]|0;
    $282 = $$0202 | 3;
    $283 = (($281) + ($282)|0);
    HEAP8[$283>>0] = $280;
    $284 = (($$2201) + 1)|0;
    $285 = (($$0202) + 4)|0;
    $286 = HEAP32[$254>>2]|0;
    $287 = HEAP32[$256>>2]|0;
    $288 = $286 << 2;
    $289 = Math_imul($288, $287)|0;
    $290 = ($285|0)<($289|0);
    if ($290) {
     $$0202 = $285;$$2201 = $284;
    } else {
     break;
    }
   }
  }
  break;
 }
 default: {
 }
 }
 _free($7);
 STACKTOP = sp;return;
}
function _LoadTextureFromImage($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$sroa$11$0$$sroa_idx8 = 0, $$sroa$5$0$$sroa_idx2 = 0, $$sroa$7$0$$sroa_idx4 = 0, $$sroa$9$0$$sroa_idx6 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP32[$1>>2]|0;
 $3 = ((($1)) + 4|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ((($1)) + 8|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = ((($1)) + 16|0);
 $8 = HEAP32[$7>>2]|0;
 $9 = ((($1)) + 12|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = (_rlglLoadTexture($2,$4,$6,$8,$10)|0);
 $12 = HEAP32[$3>>2]|0;
 $13 = HEAP32[$5>>2]|0;
 HEAP32[$0>>2] = $11;
 $$sroa$5$0$$sroa_idx2 = ((($0)) + 4|0);
 HEAP32[$$sroa$5$0$$sroa_idx2>>2] = $12;
 $$sroa$7$0$$sroa_idx4 = ((($0)) + 8|0);
 HEAP32[$$sroa$7$0$$sroa_idx4>>2] = $13;
 $$sroa$9$0$$sroa_idx6 = ((($0)) + 12|0);
 HEAP32[$$sroa$9$0$$sroa_idx6>>2] = $10;
 $$sroa$11$0$$sroa_idx8 = ((($0)) + 16|0);
 HEAP32[$$sroa$11$0$$sroa_idx8>>2] = $8;
 return;
}
function _UnloadImage($0) {
 $0 = $0|0;
 var $1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = HEAP32[$0>>2]|0;
 _free($1);
 return;
}
function _rlglLoadTexture($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $$0 = 0, $$off = 0, $$off92 = 0, $$off93 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0;
 var $26 = 0, $27 = 0, $28 = 0, $29 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0;
 var $46 = 0, $47 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond100 = 0, $or$cond7 = 0, $or$cond96 = 0, $or$cond98 = 0, $switch = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer11 = 0, $vararg_buffer15 = 0, $vararg_buffer3 = 0, $vararg_buffer5 = 0, $vararg_buffer7 = 0;
 var $vararg_buffer9 = 0, $vararg_ptr13 = 0, $vararg_ptr14 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 80|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(80|0);
 $vararg_buffer15 = sp + 64|0;
 $vararg_buffer11 = sp + 48|0;
 $vararg_buffer9 = sp + 40|0;
 $vararg_buffer7 = sp + 32|0;
 $vararg_buffer5 = sp + 24|0;
 $vararg_buffer3 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $5 = sp + 68|0;
 _glBindTexture(3553,0);
 HEAP32[$5>>2] = 0;
 $6 = HEAP32[3548]|0;
 $7 = ($6|0)==(0);
 $8 = $3 & -4;
 $switch = ($8|0)==(8);
 $or$cond100 = $switch & $7;
 if ($or$cond100) {
  _TraceLog(2,3864,$vararg_buffer);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 $9 = HEAP32[3549]|0;
 $10 = ($9|0)==(0);
 $11 = ($3|0)==(12);
 $or$cond7 = $11 & $10;
 if ($or$cond7) {
  _TraceLog(2,3908,$vararg_buffer1);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 $12 = HEAP32[3550]|0;
 $13 = ($12|0)==(0);
 $$off = (($3) + -13)|0;
 $14 = ($$off>>>0)<(2);
 $or$cond = $14 & $13;
 if ($or$cond) {
  _TraceLog(2,3953,$vararg_buffer3);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 $15 = HEAP32[3551]|0;
 $16 = ($15|0)==(0);
 $$off92 = (($3) + -15)|0;
 $17 = ($$off92>>>0)<(2);
 $or$cond96 = $17 & $16;
 if ($or$cond96) {
  _TraceLog(2,3998,$vararg_buffer5);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 $18 = HEAP32[3552]|0;
 $19 = ($18|0)==(0);
 $$off93 = (($3) + -17)|0;
 $20 = ($$off93>>>0)<(2);
 $or$cond98 = $20 & $19;
 if ($or$cond98) {
  _TraceLog(2,4043,$vararg_buffer7);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 _glGenTextures(1,($5|0));
 $21 = HEAP32[$5>>2]|0;
 _glBindTexture(3553,($21|0));
 do {
  switch ($3|0) {
  case 1:  {
   _glTexImage2D(3553,0,6409,($1|0),($2|0),0,6409,5121,($0|0));
   break;
  }
  case 2:  {
   _glTexImage2D(3553,0,6410,($1|0),($2|0),0,6410,5121,($0|0));
   break;
  }
  case 3:  {
   _glTexImage2D(3553,0,6407,($1|0),($2|0),0,6407,33635,($0|0));
   break;
  }
  case 4:  {
   _glTexImage2D(3553,0,6407,($1|0),($2|0),0,6407,5121,($0|0));
   break;
  }
  case 5:  {
   _glTexImage2D(3553,0,6408,($1|0),($2|0),0,6408,32820,($0|0));
   break;
  }
  case 6:  {
   _glTexImage2D(3553,0,6408,($1|0),($2|0),0,6408,32819,($0|0));
   break;
  }
  case 7:  {
   _glTexImage2D(3553,0,6408,($1|0),($2|0),0,6408,5121,($0|0));
   break;
  }
  case 8:  {
   $22 = HEAP32[3548]|0;
   $23 = ($22|0)==(0);
   if (!($23)) {
    _LoadCompressedTexture($0,$1,$2,$4,33776);
   }
   break;
  }
  case 9:  {
   $24 = HEAP32[3548]|0;
   $25 = ($24|0)==(0);
   if (!($25)) {
    _LoadCompressedTexture($0,$1,$2,$4,33777);
   }
   break;
  }
  case 10:  {
   $26 = HEAP32[3548]|0;
   $27 = ($26|0)==(0);
   if (!($27)) {
    _LoadCompressedTexture($0,$1,$2,$4,33778);
   }
   break;
  }
  case 11:  {
   $28 = HEAP32[3548]|0;
   $29 = ($28|0)==(0);
   if (!($29)) {
    _LoadCompressedTexture($0,$1,$2,$4,33779);
   }
   break;
  }
  case 12:  {
   $30 = HEAP32[3549]|0;
   $31 = ($30|0)==(0);
   if (!($31)) {
    _LoadCompressedTexture($0,$1,$2,$4,36196);
   }
   break;
  }
  case 13:  {
   $32 = HEAP32[3550]|0;
   $33 = ($32|0)==(0);
   if (!($33)) {
    _LoadCompressedTexture($0,$1,$2,$4,37492);
   }
   break;
  }
  case 14:  {
   $34 = HEAP32[3550]|0;
   $35 = ($34|0)==(0);
   if (!($35)) {
    _LoadCompressedTexture($0,$1,$2,$4,37496);
   }
   break;
  }
  case 15:  {
   $36 = HEAP32[3551]|0;
   $37 = ($36|0)==(0);
   if (!($37)) {
    _LoadCompressedTexture($0,$1,$2,$4,35840);
   }
   break;
  }
  case 16:  {
   $38 = HEAP32[3551]|0;
   $39 = ($38|0)==(0);
   if (!($39)) {
    _LoadCompressedTexture($0,$1,$2,$4,35842);
   }
   break;
  }
  case 17:  {
   $40 = HEAP32[3552]|0;
   $41 = ($40|0)==(0);
   if (!($41)) {
    _LoadCompressedTexture($0,$1,$2,$4,37808);
   }
   break;
  }
  case 18:  {
   $42 = HEAP32[3552]|0;
   $43 = ($42|0)==(0);
   if (!($43)) {
    _LoadCompressedTexture($0,$1,$2,$4,37815);
   }
   break;
  }
  default: {
   _TraceLog(2,4088,$vararg_buffer9);
  }
  }
 } while(0);
 $44 = HEAP32[3553]|0;
 $45 = ($44|0)==(0);
 if ($45) {
  _glTexParameteri(3553,10242,33071);
  _glTexParameteri(3553,10243,33071);
 } else {
  _glTexParameteri(3553,10242,10497);
  _glTexParameteri(3553,10243,10497);
 }
 _glTexParameteri(3553,10240,9728);
 _glTexParameteri(3553,10241,9728);
 _glBindTexture(3553,0);
 $46 = HEAP32[$5>>2]|0;
 $47 = ($46|0)==(0);
 if ($47) {
  _TraceLog(2,4166,$vararg_buffer15);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 } else {
  HEAP32[$vararg_buffer11>>2] = $46;
  $vararg_ptr13 = ((($vararg_buffer11)) + 4|0);
  HEAP32[$vararg_ptr13>>2] = $1;
  $vararg_ptr14 = ((($vararg_buffer11)) + 8|0);
  HEAP32[$vararg_ptr14>>2] = $2;
  _TraceLog(0,4117,$vararg_buffer11);
  $$0 = HEAP32[$5>>2]|0;
  STACKTOP = sp;return ($$0|0);
 }
 return (0)|0;
}
function _LoadCompressedTexture($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $$ = 0, $$03645 = 0, $$03744 = 0, $$038 = 0, $$03943 = 0, $$046 = 0, $$140 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0;
 var $23 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond42 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 _glPixelStorei(3317,1);
 switch ($4|0) {
 case 33776: case 33777: case 36196: case 37492:  {
  $$038 = 8;
  break;
 }
 default: {
  $$038 = 16;
 }
 }
 $5 = ($3|0)<(1);
 $6 = $1 | $2;
 $7 = ($6|0)==(0);
 $or$cond42 = $5 | $7;
 if ($or$cond42) {
  return;
 } else {
  $$03645 = 0;$$03744 = 0;$$03943 = $2;$$046 = $1;
 }
 while(1) {
  $8 = (($$046) + 3)|0;
  $9 = (($8|0) / 4)&-1;
  $10 = (($$03943) + 3)|0;
  $11 = (($10|0) / 4)&-1;
  $12 = Math_imul($11, $$038)|0;
  $13 = Math_imul($12, $9)|0;
  $14 = (($0) + ($$03744)|0);
  _glCompressedTexImage2D(3553,($$03645|0),($4|0),($$046|0),($$03943|0),0,($13|0),($14|0));
  $15 = (($13) + ($$03744))|0;
  $16 = (($$046|0) / 2)&-1;
  $17 = (($$03943|0) / 2)&-1;
  $18 = ($$046|0)<(2);
  $$ = $18 ? 1 : $16;
  $19 = ($$03943|0)<(2);
  $$140 = $19 ? 1 : $17;
  $20 = (($$03645) + 1)|0;
  $21 = ($20|0)>=($3|0);
  $22 = $$ | $$140;
  $23 = ($22|0)==(0);
  $or$cond = $21 | $23;
  if ($or$cond) {
   break;
  } else {
   $$03645 = $20;$$03744 = $15;$$03943 = $$140;$$046 = $$;
  }
 }
 return;
}
function _GetImageData($0) {
 $0 = $0|0;
 var $$0104105 = 0, $$0106 = 0, $$1 = 0, $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0.0, $103 = 0.0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0;
 var $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0;
 var $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0.0, $4 = 0, $40 = 0.0, $41 = 0;
 var $42 = 0, $43 = 0, $44 = 0, $45 = 0.0, $46 = 0.0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0.0, $52 = 0.0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0;
 var $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0.0, $65 = 0.0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0.0, $71 = 0.0, $72 = 0, $73 = 0, $74 = 0, $75 = 0.0, $76 = 0.0, $77 = 0, $78 = 0;
 var $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0.0, $86 = 0.0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0.0, $92 = 0.0, $93 = 0, $94 = 0, $95 = 0, $96 = 0;
 var $97 = 0.0, $98 = 0.0, $99 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 $1 = ((($0)) + 4|0);
 $2 = HEAP32[$1>>2]|0;
 $3 = ((($0)) + 8|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = $2 << 2;
 $6 = Math_imul($5, $4)|0;
 $7 = (_malloc($6)|0);
 $8 = HEAP32[$1>>2]|0;
 $9 = Math_imul($4, $8)|0;
 $10 = ($9|0)>(0);
 if (!($10)) {
  STACKTOP = sp;return ($7|0);
 }
 $11 = ((($0)) + 16|0);
 $12 = HEAP32[$11>>2]|0;
 $13 = HEAP32[$0>>2]|0;
 $$0104105 = 0;$$0106 = 0;
 while(1) {
  switch ($12|0) {
  case 1:  {
   $14 = (($13) + ($$0106)|0);
   $15 = HEAP8[$14>>0]|0;
   $16 = (($7) + ($$0104105<<2)|0);
   HEAP8[$16>>0] = $15;
   $17 = HEAP8[$14>>0]|0;
   $18 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$18>>0] = $17;
   $19 = HEAP8[$14>>0]|0;
   $20 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$20>>0] = $19;
   $21 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$21>>0] = -1;
   $22 = (($$0106) + 1)|0;
   $$1 = $22;
   break;
  }
  case 2:  {
   $23 = (($13) + ($$0106)|0);
   $24 = HEAP8[$23>>0]|0;
   $25 = (($7) + ($$0104105<<2)|0);
   HEAP8[$25>>0] = $24;
   $26 = HEAP8[$23>>0]|0;
   $27 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$27>>0] = $26;
   $28 = HEAP8[$23>>0]|0;
   $29 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$29>>0] = $28;
   $30 = (($$0106) + 1)|0;
   $31 = (($13) + ($30)|0);
   $32 = HEAP8[$31>>0]|0;
   $33 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$33>>0] = $32;
   $34 = (($$0106) + 2)|0;
   $$1 = $34;
   break;
  }
  case 5:  {
   $35 = (($13) + ($$0106<<1)|0);
   $36 = HEAP16[$35>>1]|0;
   $37 = $36&65535;
   $38 = $37 >>> 11;
   $39 = (+($38|0));
   $40 = $39 * 8.0;
   $41 = (~~(($40))&255);
   $42 = (($7) + ($$0104105<<2)|0);
   HEAP8[$42>>0] = $41;
   $43 = $37 >>> 6;
   $44 = $43 & 31;
   $45 = (+($44|0));
   $46 = $45 * 8.0;
   $47 = (~~(($46))&255);
   $48 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$48>>0] = $47;
   $49 = $37 >>> 1;
   $50 = $49 & 31;
   $51 = (+($50|0));
   $52 = $51 * 8.0;
   $53 = (~~(($52))&255);
   $54 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$54>>0] = $53;
   $55 = $37 & 1;
   $56 = (0 - ($55))|0;
   $57 = $56&255;
   $58 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$58>>0] = $57;
   $59 = (($$0106) + 1)|0;
   $$1 = $59;
   break;
  }
  case 3:  {
   $60 = (($13) + ($$0106<<1)|0);
   $61 = HEAP16[$60>>1]|0;
   $62 = $61&65535;
   $63 = $62 >>> 11;
   $64 = (+($63|0));
   $65 = $64 * 8.0;
   $66 = (~~(($65))&255);
   $67 = (($7) + ($$0104105<<2)|0);
   HEAP8[$67>>0] = $66;
   $68 = $62 >>> 5;
   $69 = $68 & 63;
   $70 = (+($69|0));
   $71 = $70 * 4.0;
   $72 = (~~(($71))&255);
   $73 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$73>>0] = $72;
   $74 = $62 & 31;
   $75 = (+($74|0));
   $76 = $75 * 8.0;
   $77 = (~~(($76))&255);
   $78 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$78>>0] = $77;
   $79 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$79>>0] = -1;
   $80 = (($$0106) + 1)|0;
   $$1 = $80;
   break;
  }
  case 6:  {
   $81 = (($13) + ($$0106<<1)|0);
   $82 = HEAP16[$81>>1]|0;
   $83 = $82&65535;
   $84 = $83 >>> 12;
   $85 = (+($84|0));
   $86 = $85 * 17.0;
   $87 = (~~(($86))&255);
   $88 = (($7) + ($$0104105<<2)|0);
   HEAP8[$88>>0] = $87;
   $89 = $83 >>> 8;
   $90 = $89 & 15;
   $91 = (+($90|0));
   $92 = $91 * 17.0;
   $93 = (~~(($92))&255);
   $94 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$94>>0] = $93;
   $95 = $83 >>> 4;
   $96 = $95 & 15;
   $97 = (+($96|0));
   $98 = $97 * 17.0;
   $99 = (~~(($98))&255);
   $100 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$100>>0] = $99;
   $101 = $83 & 15;
   $102 = (+($101|0));
   $103 = $102 * 17.0;
   $104 = (~~(($103))&255);
   $105 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$105>>0] = $104;
   $106 = (($$0106) + 1)|0;
   $$1 = $106;
   break;
  }
  case 7:  {
   $107 = (($13) + ($$0106)|0);
   $108 = HEAP8[$107>>0]|0;
   $109 = (($7) + ($$0104105<<2)|0);
   HEAP8[$109>>0] = $108;
   $110 = (($$0106) + 1)|0;
   $111 = (($13) + ($110)|0);
   $112 = HEAP8[$111>>0]|0;
   $113 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$113>>0] = $112;
   $114 = (($$0106) + 2)|0;
   $115 = (($13) + ($114)|0);
   $116 = HEAP8[$115>>0]|0;
   $117 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$117>>0] = $116;
   $118 = (($$0106) + 3)|0;
   $119 = (($13) + ($118)|0);
   $120 = HEAP8[$119>>0]|0;
   $121 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$121>>0] = $120;
   $122 = (($$0106) + 4)|0;
   $$1 = $122;
   break;
  }
  case 4:  {
   $123 = (($13) + ($$0106)|0);
   $124 = HEAP8[$123>>0]|0;
   $125 = (($7) + ($$0104105<<2)|0);
   HEAP8[$125>>0] = $124;
   $126 = (($$0106) + 1)|0;
   $127 = (($13) + ($126)|0);
   $128 = HEAP8[$127>>0]|0;
   $129 = (((($7) + ($$0104105<<2)|0)) + 1|0);
   HEAP8[$129>>0] = $128;
   $130 = (($$0106) + 2)|0;
   $131 = (($13) + ($130)|0);
   $132 = HEAP8[$131>>0]|0;
   $133 = (((($7) + ($$0104105<<2)|0)) + 2|0);
   HEAP8[$133>>0] = $132;
   $134 = (((($7) + ($$0104105<<2)|0)) + 3|0);
   HEAP8[$134>>0] = -1;
   $135 = (($$0106) + 3)|0;
   $$1 = $135;
   break;
  }
  default: {
   _TraceLog(2,4249,$vararg_buffer);
   $$1 = $$0106;
  }
  }
  $136 = (($$0104105) + 1)|0;
  $137 = HEAP32[$1>>2]|0;
  $138 = HEAP32[$3>>2]|0;
  $139 = Math_imul($138, $137)|0;
  $140 = ($136|0)<($139|0);
  if ($140) {
   $$0104105 = $136;$$0106 = $$1;
  } else {
   break;
  }
 }
 STACKTOP = sp;return ($7|0);
}
function _ErrorCallback($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $vararg_buffer = 0, $vararg_ptr1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 HEAP32[$vararg_buffer>>2] = $0;
 $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
 HEAP32[$vararg_ptr1>>2] = $1;
 _TraceLog(2,8115,$vararg_buffer);
 STACKTOP = sp;return;
}
function _rlGetVersion() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 return 4;
}
function _SetupFramebufferSize($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$sink = 0, $$sink1 = 0, $10 = 0.0, $11 = 0.0, $12 = 0.0, $13 = 0, $14 = 0.0, $15 = 0.0, $16 = 0, $17 = 0, $18 = 0.0, $19 = 0.0, $2 = 0, $20 = 0, $21 = 0, $22 = 0.0, $23 = 0, $24 = 0, $25 = 0, $26 = 0.0;
 var $27 = 0, $28 = 0.0, $29 = 0.0, $3 = 0, $30 = 0, $31 = 0, $32 = 0.0, $33 = 0.0, $34 = 0.0, $35 = 0, $36 = 0.0, $37 = 0, $38 = 0.0, $39 = 0.0, $4 = 0, $40 = 0, $41 = 0.0, $42 = 0, $43 = 0, $44 = 0.0;
 var $45 = 0, $46 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0.0, $9 = 0, $or$cond = 0, $roundf = 0.0, $roundf38 = 0.0, $roundf39 = 0.0, $roundf40 = 0.0, $vararg_buffer = 0, $vararg_buffer4 = 0, $vararg_buffer8 = 0, $vararg_ptr1 = 0, $vararg_ptr11 = 0, $vararg_ptr12 = 0, $vararg_ptr13 = 0, $vararg_ptr2 = 0;
 var $vararg_ptr3 = 0, $vararg_ptr7 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 112|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(112|0);
 $vararg_buffer8 = sp + 24|0;
 $vararg_buffer4 = sp + 16|0;
 $vararg_buffer = sp;
 $2 = sp + 40|0;
 $3 = HEAP32[3532]|0;
 $4 = ($3|0)>($0|0);
 if (!($4)) {
  $5 = HEAP32[3531]|0;
  $6 = ($5|0)>($1|0);
  if (!($6)) {
   $30 = ($3|0)<($0|0);
   $31 = ($5|0)<($1|0);
   $or$cond = $30 | $31;
   if (!($or$cond)) {
    HEAP32[3573] = $3;
    HEAP32[3574] = $5;
    HEAP32[3575] = 0;
    HEAP32[3576] = 0;
    STACKTOP = sp;return;
   }
   HEAP32[$vararg_buffer8>>2] = $3;
   $vararg_ptr11 = ((($vararg_buffer8)) + 4|0);
   HEAP32[$vararg_ptr11>>2] = $5;
   $vararg_ptr12 = ((($vararg_buffer8)) + 8|0);
   HEAP32[$vararg_ptr12>>2] = $0;
   $vararg_ptr13 = ((($vararg_buffer8)) + 12|0);
   HEAP32[$vararg_ptr13>>2] = $1;
   _TraceLog(0,8049,$vararg_buffer8);
   $32 = (+($0|0));
   $33 = (+($1|0));
   $34 = $32 / $33;
   $35 = HEAP32[3532]|0;
   $36 = (+($35|0));
   $37 = HEAP32[3531]|0;
   $38 = (+($37|0));
   $39 = $36 / $38;
   $40 = !($34 <= $39);
   if ($40) {
    $44 = $34 * $38;
    $roundf = (+_roundf((+$44)));
    $45 = (~~(($roundf)));
    HEAP32[3573] = $45;
    HEAP32[3574] = $37;
    $46 = (($45) - ($35))|0;
    HEAP32[3575] = $46;
    $$sink1 = 0;
   } else {
    HEAP32[3573] = $35;
    $41 = $36 / $34;
    $roundf38 = (+_roundf((+$41)));
    $42 = (~~(($roundf38)));
    HEAP32[3574] = $42;
    HEAP32[3575] = 0;
    $43 = (($42) - ($37))|0;
    $$sink1 = $43;
   }
   HEAP32[3576] = $$sink1;
   STACKTOP = sp;return;
  }
 }
 $7 = HEAP32[3531]|0;
 HEAP32[$vararg_buffer>>2] = $3;
 $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
 HEAP32[$vararg_ptr1>>2] = $7;
 $vararg_ptr2 = ((($vararg_buffer)) + 8|0);
 HEAP32[$vararg_ptr2>>2] = $0;
 $vararg_ptr3 = ((($vararg_buffer)) + 12|0);
 HEAP32[$vararg_ptr3>>2] = $1;
 _TraceLog(2,7906,$vararg_buffer);
 $8 = (+($0|0));
 $9 = HEAP32[3532]|0;
 $10 = (+($9|0));
 $11 = $8 / $10;
 $12 = (+($1|0));
 $13 = HEAP32[3531]|0;
 $14 = (+($13|0));
 $15 = $12 / $14;
 $16 = !($11 <= $15);
 if ($16) {
  $22 = $10 * $15;
  $roundf39 = (+_roundf((+$22)));
  $23 = (~~(($roundf39)));
  HEAP32[3573] = $23;
  HEAP32[3574] = $1;
  $24 = (($0) - ($23))|0;
  HEAP32[3575] = $24;
  $$sink = 0;
 } else {
  HEAP32[3573] = $0;
  $17 = HEAP32[3531]|0;
  $18 = (+($17|0));
  $19 = $11 * $18;
  $roundf40 = (+_roundf((+$19)));
  $20 = (~~(($roundf40)));
  HEAP32[3574] = $20;
  HEAP32[3575] = 0;
  $21 = (($1) - ($20))|0;
  $$sink = $21;
 }
 HEAP32[3576] = $$sink;
 $25 = HEAP32[3573]|0;
 $26 = (+($25|0));
 $27 = HEAP32[3532]|0;
 $28 = (+($27|0));
 $29 = $26 / $28;
 _MatrixScale($2,$29,$29,$29);
 dest=14216; src=$2; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 HEAP32[3573] = $0;
 HEAP32[3574] = $1;
 HEAP32[$vararg_buffer4>>2] = $0;
 $vararg_ptr7 = ((($vararg_buffer4)) + 4|0);
 HEAP32[$vararg_ptr7>>2] = $1;
 _TraceLog(2,7984,$vararg_buffer4);
 STACKTOP = sp;return;
}
function _WindowSizeCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $3 = 0.0, $4 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 _rlViewport(0,0,$1,$2);
 _rlMatrixMode(5889);
 _rlLoadIdentity();
 $3 = (+($1|0));
 $4 = (+($2|0));
 _rlOrtho(0.0,$3,$4,0.0,0.0,1.0);
 _rlMatrixMode(5888);
 _rlLoadIdentity();
 _rlClearScreenBuffers();
 HEAP32[3532] = $1;
 HEAP32[3531] = $2;
 HEAP32[3573] = $1;
 HEAP32[3574] = $2;
 return;
}
function _CursorEnterCallback($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 return;
}
function _KeyCallback($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $5 = HEAP32[745]|0;
 $6 = ($5|0)==($1|0);
 $7 = ($3|0)==(1);
 $or$cond = $7 & $6;
 if ($or$cond) {
  _glfwSetWindowShouldClose(($0|0),1);
  return;
 }
 $8 = $3&255;
 $9 = (16939 + ($1)|0);
 HEAP8[$9>>0] = $8;
 if (!($7)) {
  return;
 }
 HEAP32[744] = $1;
 return;
}
function _MouseButtonCallback($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $$byval_copy = 0, $$sink = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0.0, $27 = 0.0;
 var $28 = 0.0, $29 = 0, $30 = 0.0, $31 = 0, $32 = 0.0, $33 = 0.0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 128|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(128|0);
 $$byval_copy = sp + 64|0;
 $4 = sp + 8|0;
 $5 = sp;
 $6 = $2&255;
 $7 = (16933 + ($1)|0);
 HEAP8[$7>>0] = $6;
 $8 = (_IsMouseButtonPressed(0)|0);
 $9 = ($8|0)==(0);
 if ($9) {
  $10 = (_IsMouseButtonReleased(0)|0);
  $11 = ($10|0)==(0);
  if (!($11)) {
   $$sink = 0;
   label = 3;
  }
 } else {
  $$sink = 1;
  label = 3;
 }
 if ((label|0) == 3) {
  HEAP32[$4>>2] = $$sink;
 }
 $12 = ((($4)) + 8|0);
 HEAP32[$12>>2] = 0;
 $13 = ((($4)) + 4|0);
 HEAP32[$13>>2] = 1;
 $14 = ((($4)) + 24|0);
 _GetMousePosition($5);
 $15 = $5;
 $16 = $15;
 $17 = HEAP32[$16>>2]|0;
 $18 = (($15) + 4)|0;
 $19 = $18;
 $20 = HEAP32[$19>>2]|0;
 $21 = $14;
 $22 = $21;
 HEAP32[$22>>2] = $17;
 $23 = (($21) + 4)|0;
 $24 = $23;
 HEAP32[$24>>2] = $20;
 $25 = (_GetScreenWidth()|0);
 $26 = (+($25|0));
 $27 = +HEAPF32[$14>>2];
 $28 = $27 / $26;
 HEAPF32[$14>>2] = $28;
 $29 = (_GetScreenHeight()|0);
 $30 = (+($29|0));
 $31 = ((($4)) + 28|0);
 $32 = +HEAPF32[$31>>2];
 $33 = $32 / $30;
 HEAPF32[$31>>2] = $33;
 dest=$$byval_copy; src=$4; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _ProcessGestureEvent($$byval_copy);
 STACKTOP = sp;return;
}
function _MouseCursorPosCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = +$1;
 $2 = +$2;
 var $$byval_copy = 0, $$sroa$0$0$$sroa_idx = 0, $$sroa$2$0$$sroa_idx1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0.0, $21 = 0.0, $22 = 0.0, $23 = 0, $24 = 0.0, $25 = 0.0, $26 = 0.0;
 var $3 = 0, $4 = 0, $5 = 0, $6 = 0.0, $7 = 0.0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 112|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(112|0);
 $$byval_copy = sp + 56|0;
 $3 = sp;
 HEAP32[$3>>2] = 2;
 $4 = ((($3)) + 8|0);
 HEAP32[$4>>2] = 0;
 $5 = ((($3)) + 4|0);
 HEAP32[$5>>2] = 1;
 $6 = $1;
 $7 = $2;
 $$sroa$0$0$$sroa_idx = ((($3)) + 24|0);
 HEAPF32[$$sroa$0$0$$sroa_idx>>2] = $6;
 $$sroa$2$0$$sroa_idx1 = ((($3)) + 28|0);
 HEAPF32[$$sroa$2$0$$sroa_idx1>>2] = $7;
 $8 = ((($3)) + 24|0);
 $9 = $8;
 $10 = $9;
 $11 = HEAP32[$10>>2]|0;
 $12 = (($9) + 4)|0;
 $13 = $12;
 $14 = HEAP32[$13>>2]|0;
 $15 = 13896;
 $16 = $15;
 HEAP32[$16>>2] = $11;
 $17 = (($15) + 4)|0;
 $18 = $17;
 HEAP32[$18>>2] = $14;
 $19 = (_GetScreenWidth()|0);
 $20 = (+($19|0));
 $21 = +HEAPF32[$8>>2];
 $22 = $21 / $20;
 HEAPF32[$8>>2] = $22;
 $23 = (_GetScreenHeight()|0);
 $24 = (+($23|0));
 $25 = +HEAPF32[$$sroa$2$0$$sroa_idx1>>2];
 $26 = $25 / $24;
 HEAPF32[$$sroa$2$0$$sroa_idx1>>2] = $26;
 dest=$$byval_copy; src=$3; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _ProcessGestureEvent($$byval_copy);
 STACKTOP = sp;return;
}
function _CharCallback($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 HEAP32[744] = $1;
 return;
}
function _ScrollCallback($0,$1,$2) {
 $0 = $0|0;
 $1 = +$1;
 $2 = +$2;
 var $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = (~~(($2)));
 HEAP32[3946] = $3;
 return;
}
function _WindowIconifyCallback($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$sink = 0, $2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ($1|0)!=(0);
 $$sink = $2&1;
 HEAP32[3945] = $$sink;
 return;
}
function _rlglInit($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$05965 = 0, $$06066 = 0, $$06167 = 0, $$062 = 0, $$sink63 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0;
 var $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0;
 var $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0.0, $72 = 0.0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0;
 var $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $9 = 0, $exitcond = 0, $exitcond69 = 0, $exitcond70 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer10 = 0, $vararg_buffer13 = 0, $vararg_buffer15 = 0, $vararg_buffer17 = 0, $vararg_buffer19 = 0;
 var $vararg_buffer21 = 0, $vararg_buffer23 = 0, $vararg_buffer25 = 0, $vararg_buffer27 = 0, $vararg_buffer29 = 0, $vararg_buffer31 = 0, $vararg_buffer34 = 0, $vararg_buffer36 = 0, $vararg_buffer39 = 0, $vararg_buffer4 = 0, $vararg_buffer41 = 0, $vararg_buffer7 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 2464|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(2464|0);
 $vararg_buffer41 = sp + 2184|0;
 $vararg_buffer39 = sp + 2176|0;
 $vararg_buffer36 = sp + 2168|0;
 $vararg_buffer34 = sp + 2160|0;
 $vararg_buffer31 = sp + 2152|0;
 $vararg_buffer29 = sp + 2144|0;
 $vararg_buffer27 = sp + 2136|0;
 $vararg_buffer25 = sp + 2128|0;
 $vararg_buffer23 = sp + 2120|0;
 $vararg_buffer21 = sp + 2112|0;
 $vararg_buffer19 = sp + 2104|0;
 $vararg_buffer17 = sp + 2096|0;
 $vararg_buffer15 = sp + 2088|0;
 $vararg_buffer13 = sp + 2080|0;
 $vararg_buffer10 = sp + 2072|0;
 $vararg_buffer7 = sp + 24|0;
 $vararg_buffer4 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $2 = sp + 2400|0;
 $3 = sp + 2384|0;
 $4 = sp + 2320|0;
 $5 = sp + 2256|0;
 $6 = sp + 2192|0;
 $7 = (_glGetString(7936)|0);
 HEAP32[$vararg_buffer>>2] = $7;
 _TraceLog(0,4547,$vararg_buffer);
 $8 = (_glGetString(7937)|0);
 HEAP32[$vararg_buffer1>>2] = $8;
 _TraceLog(0,4565,$vararg_buffer1);
 $9 = (_glGetString(7938)|0);
 HEAP32[$vararg_buffer4>>2] = $9;
 _TraceLog(0,4583,$vararg_buffer4);
 $10 = (_glGetString(35724)|0);
 HEAP32[$vararg_buffer7>>2] = $10;
 _TraceLog(0,4601,$vararg_buffer7);
 $11 = (_glGetString(7939)|0);
 $12 = (_strlen($11)|0);
 $13 = (($12) + 1)|0;
 $14 = (_malloc($13)|0);
 _memcpy(($14|0),($11|0),($13|0))|0;
 $$062 = 0;$$sink63 = $14;
 while(1) {
  $15 = (_strtok($$sink63,4619)|0);
  $16 = (($vararg_buffer7) + ($$062<<2)|0);
  HEAP32[$16>>2] = $15;
  $17 = ($15|0)==(0|0);
  $18 = (($$062) + 1)|0;
  if ($17) {
   break;
  } else {
   $$062 = $18;$$sink63 = 0;
  }
 }
 _free($14);
 $19 = (($$062) + -1)|0;
 HEAP32[$vararg_buffer10>>2] = $19;
 _TraceLog(0,4621,$vararg_buffer10);
 $20 = ($$062|0)>(1);
 if ($20) {
  $$06167 = 0;
  while(1) {
   $23 = (($vararg_buffer7) + ($$06167<<2)|0);
   $24 = HEAP32[$23>>2]|0;
   $25 = (_strcmp($24,4656)|0);
   $26 = ($25|0)==(0);
   if ($26) {
    HEAP32[3611] = 1;
    $27 = (_eglGetProcAddress((4683|0))|0);
    HEAP32[3612] = $27;
    $28 = (_eglGetProcAddress((4704|0))|0);
    HEAP32[3613] = $28;
    $29 = (_eglGetProcAddress((4725|0))|0);
    HEAP32[3614] = $29;
   }
   $30 = (_strcmp($24,4749)|0);
   $31 = ($30|0)==(0);
   if ($31) {
    HEAP32[3553] = 1;
   }
   $32 = (_strcmp($24,4769)|0);
   $33 = ($32|0)==(0);
   if ($33) {
    label = 12;
   } else {
    $34 = HEAP32[$23>>2]|0;
    $35 = (_strcmp($34,4801)|0);
    $36 = ($35|0)==(0);
    if ($36) {
     label = 12;
    } else {
     $37 = (_strcmp($34,4834)|0);
     $38 = ($37|0)==(0);
     if ($38) {
      label = 12;
     }
    }
   }
   if ((label|0) == 12) {
    label = 0;
    HEAP32[3548] = 1;
   }
   $39 = (_strcmp($24,4874)|0);
   $40 = ($39|0)==(0);
   if ($40) {
    label = 15;
   } else {
    $41 = HEAP32[$23>>2]|0;
    $42 = (_strcmp($41,4910)|0);
    $43 = ($42|0)==(0);
    if ($43) {
     label = 15;
    }
   }
   if ((label|0) == 15) {
    label = 0;
    HEAP32[3549] = 1;
   }
   $44 = HEAP32[$23>>2]|0;
   $45 = (_strcmp($44,4943)|0);
   $46 = ($45|0)==(0);
   if ($46) {
    HEAP32[3550] = 1;
   }
   $47 = (_strcmp($44,4968)|0);
   $48 = ($47|0)==(0);
   if ($48) {
    HEAP32[3551] = 1;
   }
   $49 = (_strcmp($44,5001)|0);
   $50 = ($49|0)==(0);
   if ($50) {
    HEAP32[3552] = 1;
   }
   $51 = (_strcmp($44,5037)|0);
   $52 = ($51|0)==(0);
   if ($52) {
    HEAP32[3615] = 1;
    _glGetFloatv(34047,(14464|0));
   }
   $53 = HEAP32[$23>>2]|0;
   $54 = (_strcmp($53,5071)|0);
   $55 = ($54|0)==(0);
   if ($55) {
    HEAP32[3617] = 1;
   }
   $56 = (($$06167) + 1)|0;
   $exitcond70 = ($56|0)==($19|0);
   if ($exitcond70) {
    break;
   } else {
    $$06167 = $56;
   }
  }
 }
 $21 = HEAP32[3611]|0;
 $22 = ($21|0)==(0);
 if ($22) {
  _TraceLog(2,5174,$vararg_buffer15);
 } else {
  _TraceLog(0,5099,$vararg_buffer13);
 }
 $57 = HEAP32[3553]|0;
 $58 = ($57|0)==(0);
 if ($58) {
  _TraceLog(2,5310,$vararg_buffer19);
 } else {
  _TraceLog(0,5235,$vararg_buffer17);
 }
 $59 = HEAP32[3548]|0;
 $60 = ($59|0)==(0);
 if (!($60)) {
  _TraceLog(0,5402,$vararg_buffer21);
 }
 $61 = HEAP32[3549]|0;
 $62 = ($61|0)==(0);
 if (!($62)) {
  _TraceLog(0,5448,$vararg_buffer23);
 }
 $63 = HEAP32[3550]|0;
 $64 = ($63|0)==(0);
 if (!($64)) {
  _TraceLog(0,5495,$vararg_buffer25);
 }
 $65 = HEAP32[3551]|0;
 $66 = ($65|0)==(0);
 if (!($66)) {
  _TraceLog(0,5546,$vararg_buffer27);
 }
 $67 = HEAP32[3552]|0;
 $68 = ($67|0)==(0);
 if (!($68)) {
  _TraceLog(0,5593,$vararg_buffer29);
 }
 $69 = HEAP32[3615]|0;
 $70 = ($69|0)==(0);
 if (!($70)) {
  $71 = +HEAPF32[3616];
  $72 = $71;
  HEAPF64[$vararg_buffer31>>3] = $72;
  _TraceLog(0,5640,$vararg_buffer31);
 }
 $73 = HEAP32[3617]|0;
 $74 = ($73|0)==(0);
 if (!($74)) {
  _TraceLog(0,5706,$vararg_buffer34);
 }
 HEAP32[$vararg_buffer10>>2] = -1;
 $75 = (_rlglLoadTexture($vararg_buffer10,1,1,7,1)|0);
 HEAP32[3618] = $75;
 $76 = ($75|0)==(0);
 if ($76) {
  _TraceLog(2,5810,$vararg_buffer39);
 } else {
  HEAP32[$vararg_buffer36>>2] = $75;
  _TraceLog(0,5759,$vararg_buffer36);
 }
 _LoadDefaultShader($2);
 dest=14476; src=$2; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=14532; src=$2; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _LoadDefaultBuffers();
 $77 = (_malloc(49152)|0);
 HEAP32[3647] = $77;
 $$06066 = 0;
 while(1) {
  $79 = HEAP32[3647]|0;
  $80 = (($79) + (($$06066*12)|0)|0);
  _VectorZero($3);
  ;HEAP32[$80>>2]=HEAP32[$3>>2]|0;HEAP32[$80+4>>2]=HEAP32[$3+4>>2]|0;HEAP32[$80+8>>2]=HEAP32[$3+8>>2]|0;
  $81 = (($$06066) + 1)|0;
  $exitcond69 = ($81|0)==(4096);
  if ($exitcond69) {
   break;
  } else {
   $$06066 = $81;
  }
 }
 $78 = (_malloc(36864)|0);
 HEAP32[3648] = $78;
 $$05965 = 0;
 while(1) {
  $82 = (((($78) + (($$05965*144)|0)|0)) + 8|0);
  HEAP32[$82>>2] = 0;
  $83 = (($78) + (($$05965*144)|0)|0);
  HEAP32[$83>>2] = 0;
  $84 = (($$05965) + 1)|0;
  $exitcond = ($84|0)==(256);
  if ($exitcond) {
   break;
  } else {
   $$05965 = $84;
  }
 }
 HEAP32[3649] = 1;
 $85 = HEAP32[3618]|0;
 $86 = ((($78)) + 8|0);
 HEAP32[$86>>2] = $85;
 HEAP32[3650] = 4;
 _MatrixIdentity($4);
 dest=14604; src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14668); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14732); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14796); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14860); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14924); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(14988); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15052); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15116); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15180); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15244); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15308); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15372); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15436); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15500); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($4);
 dest=(15564); src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($5);
 dest=14312; src=$5; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixIdentity($6);
 dest=14376; src=$6; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 HEAP32[3577] = 14376;
 _glDepthFunc(515);
 _glDisable(2929);
 _glBlendFunc(770,771);
 _glEnable(3042);
 _glCullFace(1029);
 _glFrontFace(2305);
 _glEnable(2884);
 _glClearColor(0.0,0.0,0.0,1.0);
 _glClearDepthf(1.0);
 _glClear(16640);
 HEAP32[3907] = $0;
 HEAP32[3908] = $1;
 _TraceLog(0,5849,$vararg_buffer41);
 STACKTOP = sp;return;
}
function _SetupViewport() {
 var $0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3575]|0;
 $1 = (($0|0) / 2)&-1;
 $2 = HEAP32[3576]|0;
 $3 = (($2|0) / 2)&-1;
 $4 = HEAP32[3573]|0;
 $5 = (($4) - ($0))|0;
 $6 = HEAP32[3574]|0;
 $7 = (($6) - ($2))|0;
 _rlViewport($1,$3,$5,$7);
 return;
}
function _rlMatrixMode($0) {
 $0 = $0|0;
 var $modelview$sink = 0, label = 0, sp = 0;
 sp = STACKTOP;
 switch ($0|0) {
 case 5889:  {
  $modelview$sink = 14312;
  label = 3;
  break;
 }
 case 5888:  {
  $modelview$sink = 14376;
  label = 3;
  break;
 }
 default: {
 }
 }
 if ((label|0) == 3) {
  HEAP32[3577] = $modelview$sink;
 }
 HEAP32[3610] = $0;
 return;
}
function _rlLoadIdentity() {
 var $0 = 0, $1 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $0 = sp;
 $1 = HEAP32[3577]|0;
 _MatrixIdentity($0);
 dest=$1; src=$0; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _rlOrtho($0,$1,$2,$3,$4,$5) {
 $0 = +$0;
 $1 = +$1;
 $2 = +$2;
 $3 = +$3;
 $4 = +$4;
 $5 = +$5;
 var $$byval_copy = 0, $$byval_copy1 = 0, $6 = 0, $7 = 0, $8 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 256|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(256|0);
 $$byval_copy1 = sp + 192|0;
 $$byval_copy = sp + 128|0;
 $6 = sp + 64|0;
 $7 = sp;
 _MatrixOrtho($6,$0,$1,$2,$3,$4,$5);
 _MatrixTranspose($6);
 $8 = HEAP32[3577]|0;
 dest=$$byval_copy; src=$8; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy1; src=$6; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixMultiply($7,$$byval_copy,$$byval_copy1);
 dest=$8; src=$7; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _ClearBackground($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = HEAP8[$0>>0]|0;
 $2 = ((($0)) + 1|0);
 $3 = HEAP8[$2>>0]|0;
 $4 = ((($0)) + 2|0);
 $5 = HEAP8[$4>>0]|0;
 $6 = ((($0)) + 3|0);
 $7 = HEAP8[$6>>0]|0;
 _rlClearColor($1,$3,$5,$7);
 return;
}
function _rlClearColor($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $10 = 0.0, $11 = 0.0, $4 = 0.0, $5 = 0.0, $6 = 0.0, $7 = 0.0, $8 = 0.0, $9 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $4 = (+($0&255));
 $5 = $4 / 255.0;
 $6 = (+($1&255));
 $7 = $6 / 255.0;
 $8 = (+($2&255));
 $9 = $8 / 255.0;
 $10 = (+($3&255));
 $11 = $10 / 255.0;
 _glClearColor((+$5),(+$7),(+$9),(+$11));
 return;
}
function _rlViewport($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 _glViewport(($0|0),($1|0),($2|0),($3|0));
 return;
}
function _LoadDefaultShader($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 1008|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(1008|0);
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $1 = sp + 16|0;
 $2 = sp + 513|0;
 $3 = sp + 72|0;
 _memcpy(($2|0),(6425|0),489)|0;
 _memcpy(($3|0),(6914|0),441)|0;
 $4 = (_LoadShaderProgram($2,$3)|0);
 HEAP32[$1>>2] = $4;
 $5 = ($4|0)==(0);
 if ($5) {
  HEAP32[$vararg_buffer1>>2] = $4;
  _TraceLog(2,7403,$vararg_buffer1);
 } else {
  HEAP32[$vararg_buffer>>2] = $4;
  _TraceLog(0,7355,$vararg_buffer);
 }
 $6 = HEAP32[$1>>2]|0;
 $7 = ($6|0)==(0);
 if (!($7)) {
  _LoadDefaultShaderLocations($1);
 }
 dest=$0; src=$1; stop=dest+56|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _LoadDefaultBuffers() {
 var $$05365 = 0, $$05467 = 0, $$05770 = 0, $$05972 = 0, $$066 = 0, $0 = 0, $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0;
 var $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0;
 var $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0;
 var $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0;
 var $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0;
 var $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $exitcond = 0, $exitcond75 = 0, $exitcond78 = 0, $exitcond80 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer10 = 0, $vararg_buffer14 = 0, $vararg_buffer17 = 0;
 var $vararg_buffer3 = 0, $vararg_buffer7 = 0, $vararg_ptr13 = 0, $vararg_ptr20 = 0, $vararg_ptr21 = 0, $vararg_ptr22 = 0, $vararg_ptr6 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $vararg_buffer17 = sp + 48|0;
 $vararg_buffer14 = sp + 40|0;
 $vararg_buffer10 = sp + 32|0;
 $vararg_buffer7 = sp + 24|0;
 $vararg_buffer3 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $0 = (_malloc(24576)|0);
 HEAP32[(15648)>>2] = $0;
 $1 = (_malloc(8192)|0);
 HEAP32[(15656)>>2] = $1;
 HEAP32[(15652)>>2] = 0;
 HEAP32[(15660)>>2] = 0;
 _memset(($0|0),0,24576)|0;
 $$05972 = 0;
 while(1) {
  $2 = HEAP32[(15656)>>2]|0;
  $3 = (($2) + ($$05972)|0);
  HEAP8[$3>>0] = 0;
  $4 = (($$05972) + 1)|0;
  $exitcond80 = ($4|0)==(8192);
  if ($exitcond80) {
   break;
  } else {
   $$05972 = $4;
  }
 }
 HEAP32[3909] = 0;
 HEAP32[(15644)>>2] = 0;
 HEAP32[(15640)>>2] = 0;
 $5 = (_malloc(73728)|0);
 HEAP32[(15696)>>2] = $5;
 $6 = (_malloc(24576)|0);
 HEAP32[(15704)>>2] = $6;
 HEAP32[(15700)>>2] = 0;
 HEAP32[(15708)>>2] = 0;
 _memset(($5|0),0,73728)|0;
 $$05770 = 0;
 while(1) {
  $7 = HEAP32[(15704)>>2]|0;
  $8 = (($7) + ($$05770)|0);
  HEAP8[$8>>0] = 0;
  $9 = (($$05770) + 1)|0;
  $exitcond78 = ($9|0)==(24576);
  if ($exitcond78) {
   break;
  } else {
   $$05770 = $9;
  }
 }
 HEAP32[3921] = 0;
 HEAP32[(15692)>>2] = 0;
 HEAP32[(15688)>>2] = 0;
 $10 = (_malloc(49152)|0);
 HEAP32[(15744)>>2] = $10;
 $11 = (_malloc(32768)|0);
 HEAP32[(15748)>>2] = $11;
 $12 = (_malloc(16384)|0);
 HEAP32[(15752)>>2] = $12;
 $13 = (_malloc(12288)|0);
 HEAP32[(15756)>>2] = $13;
 $14 = HEAP32[(15744)>>2]|0;
 _memset(($14|0),0,49152)|0;
 $15 = HEAP32[(15748)>>2]|0;
 _memset(($15|0),0,32768)|0;
 $$05467 = 0;
 while(1) {
  $17 = HEAP32[(15752)>>2]|0;
  $18 = (($17) + ($$05467)|0);
  HEAP8[$18>>0] = 0;
  $19 = (($$05467) + 1)|0;
  $exitcond75 = ($19|0)==(16384);
  if ($exitcond75) {
   break;
  } else {
   $$05467 = $19;
  }
 }
 $16 = HEAP32[(15756)>>2]|0;
 $$05365 = 0;$$066 = 0;
 while(1) {
  $22 = $$05365 << 2;
  $23 = $22&65535;
  $24 = (($16) + ($$066<<1)|0);
  HEAP16[$24>>1] = $23;
  $25 = $22 | 1;
  $26 = $25&65535;
  $27 = $$066 | 1;
  $28 = (($16) + ($27<<1)|0);
  HEAP16[$28>>1] = $26;
  $29 = $22 | 2;
  $30 = $29&65535;
  $31 = (($$066) + 2)|0;
  $32 = (($16) + ($31<<1)|0);
  HEAP16[$32>>1] = $30;
  $33 = (($$066) + 3)|0;
  $34 = (($16) + ($33<<1)|0);
  HEAP16[$34>>1] = $23;
  $35 = (($$066) + 4)|0;
  $36 = (($16) + ($35<<1)|0);
  HEAP16[$36>>1] = $30;
  $37 = $22 | 3;
  $38 = $37&65535;
  $39 = (($$066) + 5)|0;
  $40 = (($16) + ($39<<1)|0);
  HEAP16[$40>>1] = $38;
  $41 = (($$05365) + 1)|0;
  $42 = (($$066) + 6)|0;
  $exitcond = ($41|0)==(1024);
  if ($exitcond) {
   break;
  } else {
   $$05365 = $41;$$066 = $42;
  }
 }
 HEAP32[3933] = 0;
 HEAP32[(15736)>>2] = 0;
 HEAP32[(15740)>>2] = 0;
 _TraceLog(0,5896,$vararg_buffer);
 $20 = HEAP32[3611]|0;
 $21 = ($20|0)==(0);
 if (!($21)) {
  $43 = HEAP32[3612]|0;
  FUNCTION_TABLE_vii[$43 & 63](1,(15664));
  $44 = HEAP32[3613]|0;
  $45 = HEAP32[(15664)>>2]|0;
  FUNCTION_TABLE_vi[$44 & 31]($45);
 }
 _glGenBuffers(2,((15668)|0));
 $46 = HEAP32[(15668)>>2]|0;
 _glBindBuffer(34962,($46|0));
 $47 = HEAP32[(15648)>>2]|0;
 _glBufferData(34962,24576,($47|0),35048);
 $48 = HEAP32[(14536)>>2]|0;
 _glEnableVertexAttribArray(($48|0));
 $49 = HEAP32[(14536)>>2]|0;
 _glVertexAttribPointer(($49|0),3,5126,0,0,(0|0));
 _glGenBuffers(2,((15672)|0));
 $50 = HEAP32[(15672)>>2]|0;
 _glBindBuffer(34962,($50|0));
 $51 = HEAP32[(15656)>>2]|0;
 _glBufferData(34962,8192,($51|0),35048);
 $52 = HEAP32[(14556)>>2]|0;
 _glEnableVertexAttribArray(($52|0));
 $53 = HEAP32[(14556)>>2]|0;
 _glVertexAttribPointer(($53|0),4,5121,1,0,(0|0));
 $54 = HEAP32[3611]|0;
 $55 = ($54|0)==(0);
 if ($55) {
  $57 = HEAP32[(15668)>>2]|0;
  $58 = HEAP32[(15672)>>2]|0;
  HEAP32[$vararg_buffer3>>2] = $57;
  $vararg_ptr6 = ((($vararg_buffer3)) + 4|0);
  HEAP32[$vararg_ptr6>>2] = $58;
  _TraceLog(0,6034,$vararg_buffer3);
 } else {
  $56 = HEAP32[(15664)>>2]|0;
  HEAP32[$vararg_buffer1>>2] = $56;
  _TraceLog(0,5969,$vararg_buffer1);
 }
 $59 = HEAP32[3611]|0;
 $60 = ($59|0)==(0);
 if (!($60)) {
  $61 = HEAP32[3612]|0;
  FUNCTION_TABLE_vii[$61 & 63](1,(15712));
  $62 = HEAP32[3613]|0;
  $63 = HEAP32[(15712)>>2]|0;
  FUNCTION_TABLE_vi[$62 & 31]($63);
 }
 _glGenBuffers(1,((15716)|0));
 $64 = HEAP32[(15716)>>2]|0;
 _glBindBuffer(34962,($64|0));
 $65 = HEAP32[(15696)>>2]|0;
 _glBufferData(34962,73728,($65|0),35048);
 $66 = HEAP32[(14536)>>2]|0;
 _glEnableVertexAttribArray(($66|0));
 $67 = HEAP32[(14536)>>2]|0;
 _glVertexAttribPointer(($67|0),3,5126,0,0,(0|0));
 _glGenBuffers(1,((15720)|0));
 $68 = HEAP32[(15720)>>2]|0;
 _glBindBuffer(34962,($68|0));
 $69 = HEAP32[(15704)>>2]|0;
 _glBufferData(34962,24576,($69|0),35048);
 $70 = HEAP32[(14556)>>2]|0;
 _glEnableVertexAttribArray(($70|0));
 $71 = HEAP32[(14556)>>2]|0;
 _glVertexAttribPointer(($71|0),4,5121,1,0,(0|0));
 $72 = HEAP32[3611]|0;
 $73 = ($72|0)==(0);
 if ($73) {
  $75 = HEAP32[(15716)>>2]|0;
  $76 = HEAP32[(15720)>>2]|0;
  HEAP32[$vararg_buffer10>>2] = $75;
  $vararg_ptr13 = ((($vararg_buffer10)) + 4|0);
  HEAP32[$vararg_ptr13>>2] = $76;
  _TraceLog(0,6180,$vararg_buffer10);
 } else {
  $74 = HEAP32[(15712)>>2]|0;
  HEAP32[$vararg_buffer7>>2] = $74;
  _TraceLog(0,6111,$vararg_buffer7);
 }
 $77 = HEAP32[3611]|0;
 $78 = ($77|0)==(0);
 if (!($78)) {
  $79 = HEAP32[3612]|0;
  FUNCTION_TABLE_vii[$79 & 63](1,(15760));
  $80 = HEAP32[3613]|0;
  $81 = HEAP32[(15760)>>2]|0;
  FUNCTION_TABLE_vi[$80 & 31]($81);
 }
 _glGenBuffers(1,((15764)|0));
 $82 = HEAP32[(15764)>>2]|0;
 _glBindBuffer(34962,($82|0));
 $83 = HEAP32[(15744)>>2]|0;
 _glBufferData(34962,49152,($83|0),35048);
 $84 = HEAP32[(14536)>>2]|0;
 _glEnableVertexAttribArray(($84|0));
 $85 = HEAP32[(14536)>>2]|0;
 _glVertexAttribPointer(($85|0),3,5126,0,0,(0|0));
 _glGenBuffers(1,((15768)|0));
 $86 = HEAP32[(15768)>>2]|0;
 _glBindBuffer(34962,($86|0));
 $87 = HEAP32[(15748)>>2]|0;
 _glBufferData(34962,32768,($87|0),35048);
 $88 = HEAP32[(14540)>>2]|0;
 _glEnableVertexAttribArray(($88|0));
 $89 = HEAP32[(14540)>>2]|0;
 _glVertexAttribPointer(($89|0),2,5126,0,0,(0|0));
 _glGenBuffers(1,((15772)|0));
 $90 = HEAP32[(15772)>>2]|0;
 _glBindBuffer(34962,($90|0));
 $91 = HEAP32[(15752)>>2]|0;
 _glBufferData(34962,16384,($91|0),35048);
 $92 = HEAP32[(14556)>>2]|0;
 _glEnableVertexAttribArray(($92|0));
 $93 = HEAP32[(14556)>>2]|0;
 _glVertexAttribPointer(($93|0),4,5121,1,0,(0|0));
 _glGenBuffers(1,((15776)|0));
 $94 = HEAP32[(15776)>>2]|0;
 _glBindBuffer(34963,($94|0));
 $95 = HEAP32[(15756)>>2]|0;
 _glBufferData(34963,12288,($95|0),35044);
 $96 = HEAP32[3611]|0;
 $97 = ($96|0)==(0);
 if ($97) {
  $99 = HEAP32[(15764)>>2]|0;
  $100 = HEAP32[(15768)>>2]|0;
  $101 = HEAP32[(15772)>>2]|0;
  $102 = HEAP32[(15776)>>2]|0;
  HEAP32[$vararg_buffer17>>2] = $99;
  $vararg_ptr20 = ((($vararg_buffer17)) + 4|0);
  HEAP32[$vararg_ptr20>>2] = $100;
  $vararg_ptr21 = ((($vararg_buffer17)) + 8|0);
  HEAP32[$vararg_ptr21>>2] = $101;
  $vararg_ptr22 = ((($vararg_buffer17)) + 12|0);
  HEAP32[$vararg_ptr22>>2] = $102;
  _TraceLog(0,6326,$vararg_buffer17);
 } else {
  $98 = HEAP32[(15760)>>2]|0;
  HEAP32[$vararg_buffer14>>2] = $98;
  _TraceLog(0,6261,$vararg_buffer14);
 }
 $103 = HEAP32[3611]|0;
 $104 = ($103|0)==(0);
 if ($104) {
  STACKTOP = sp;return;
 }
 $105 = HEAP32[3613]|0;
 FUNCTION_TABLE_vi[$105 & 31](0);
 STACKTOP = sp;return;
}
function _LoadShaderProgram($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $$alloca_mul = 0, $$alloca_mul34 = 0, $$alloca_mul36 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0;
 var $25 = 0, $26 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer10 = 0, $vararg_buffer13 = 0, $vararg_buffer16 = 0, $vararg_buffer19 = 0, $vararg_buffer22 = 0, $vararg_buffer4 = 0, $vararg_buffer7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 96|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(96|0);
 $vararg_buffer22 = sp + 64|0;
 $vararg_buffer19 = sp + 56|0;
 $vararg_buffer16 = sp + 48|0;
 $vararg_buffer13 = sp + 40|0;
 $vararg_buffer10 = sp + 32|0;
 $vararg_buffer7 = sp + 24|0;
 $vararg_buffer4 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $2 = sp + 80|0;
 $3 = sp + 76|0;
 $4 = sp + 72|0;
 $5 = sp + 68|0;
 $6 = (_glCreateShader(35633)|0);
 $7 = (_glCreateShader(35632)|0);
 HEAP32[$2>>2] = $0;
 HEAP32[$3>>2] = $1;
 _glShaderSource(($6|0),1,($2|0),(0|0));
 _glShaderSource(($7|0),1,($3|0),(0|0));
 HEAP32[$4>>2] = 0;
 _glCompileShader(($6|0));
 _glGetShaderiv(($6|0),35713,($4|0));
 $8 = HEAP32[$4>>2]|0;
 $9 = ($8|0)==(1);
 if ($9) {
  HEAP32[$vararg_buffer4>>2] = $6;
  _TraceLog(0,7659,$vararg_buffer4);
 } else {
  HEAP32[$vararg_buffer>>2] = $6;
  _TraceLog(2,7607,$vararg_buffer);
  HEAP32[$vararg_buffer>>2] = 0;
  _glGetShaderiv(($6|0),35716,($vararg_buffer|0));
  $10 = HEAP32[$vararg_buffer>>2]|0;
  $11 = (_llvm_stacksave()|0);
  $$alloca_mul = $10;
  $12 = STACKTOP; STACKTOP = STACKTOP + ((((1*$$alloca_mul)|0)+15)&-16)|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(((((1*$$alloca_mul)|0)+15)&-16)|0);;
  $13 = HEAP32[$vararg_buffer>>2]|0;
  _glGetShaderInfoLog(($6|0),($13|0),($5|0),($12|0));
  HEAP32[$vararg_buffer1>>2] = $12;
  _TraceLog(0,7656,$vararg_buffer1);
  _llvm_stackrestore(($11|0));
 }
 _glCompileShader(($7|0));
 _glGetShaderiv(($7|0),35713,($4|0));
 $14 = HEAP32[$4>>2]|0;
 $15 = ($14|0)==(1);
 if ($15) {
  HEAP32[$vararg_buffer13>>2] = $7;
  _TraceLog(0,7760,$vararg_buffer13);
 } else {
  HEAP32[$vararg_buffer7>>2] = $7;
  _TraceLog(2,7709,$vararg_buffer7);
  HEAP32[$vararg_buffer7>>2] = 0;
  _glGetShaderiv(($7|0),35716,($vararg_buffer7|0));
  $16 = HEAP32[$vararg_buffer7>>2]|0;
  $17 = (_llvm_stacksave()|0);
  $$alloca_mul34 = $16;
  $18 = STACKTOP; STACKTOP = STACKTOP + ((((1*$$alloca_mul34)|0)+15)&-16)|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(((((1*$$alloca_mul34)|0)+15)&-16)|0);;
  $19 = HEAP32[$vararg_buffer7>>2]|0;
  _glGetShaderInfoLog(($7|0),($19|0),($5|0),($18|0));
  HEAP32[$vararg_buffer10>>2] = $18;
  _TraceLog(0,7656,$vararg_buffer10);
  _llvm_stackrestore(($17|0));
 }
 $20 = (_glCreateProgram()|0);
 _glAttachShader(($20|0),($6|0));
 _glAttachShader(($20|0),($7|0));
 _glBindAttribLocation(($20|0),0,(7451|0));
 _glBindAttribLocation(($20|0),1,(7466|0));
 _glBindAttribLocation(($20|0),2,(7497|0));
 _glBindAttribLocation(($20|0),3,(7524|0));
 _glBindAttribLocation(($20|0),4,(7510|0));
 _glBindAttribLocation(($20|0),5,(7481|0));
 _glLinkProgram(($20|0));
 _glGetProgramiv(($20|0),35714,($4|0));
 $21 = HEAP32[$4>>2]|0;
 $22 = ($21|0)==(0);
 if ($22) {
  HEAP32[$vararg_buffer16>>2] = $20;
  _TraceLog(2,7812,$vararg_buffer16);
  HEAP32[$vararg_buffer16>>2] = 0;
  _glGetProgramiv(($20|0),35716,($vararg_buffer16|0));
  $23 = HEAP32[$vararg_buffer16>>2]|0;
  $24 = (_llvm_stacksave()|0);
  $$alloca_mul36 = $23;
  $25 = STACKTOP; STACKTOP = STACKTOP + ((((1*$$alloca_mul36)|0)+15)&-16)|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(((((1*$$alloca_mul36)|0)+15)&-16)|0);;
  $26 = HEAP32[$vararg_buffer16>>2]|0;
  _glGetProgramInfoLog(($20|0),($26|0),($5|0),($25|0));
  HEAP32[$vararg_buffer19>>2] = $25;
  _TraceLog(0,7656,$vararg_buffer19);
  _glDeleteProgram(($20|0));
  _llvm_stackrestore(($24|0));
  $$0 = 0;
  _glDeleteShader(($6|0));
  _glDeleteShader(($7|0));
  STACKTOP = sp;return ($$0|0);
 } else {
  HEAP32[$vararg_buffer22>>2] = $20;
  _TraceLog(0,7858,$vararg_buffer22);
  $$0 = $20;
  _glDeleteShader(($6|0));
  _glDeleteShader(($7|0));
  STACKTOP = sp;return ($$0|0);
 }
 return (0)|0;
}
function _LoadDefaultShaderLocations($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0;
 var $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0;
 var sp = 0;
 sp = STACKTOP;
 $1 = HEAP32[$0>>2]|0;
 $2 = (_glGetAttribLocation(($1|0),(7451|0))|0);
 $3 = ((($0)) + 4|0);
 HEAP32[$3>>2] = $2;
 $4 = HEAP32[$0>>2]|0;
 $5 = (_glGetAttribLocation(($4|0),(7466|0))|0);
 $6 = ((($0)) + 8|0);
 HEAP32[$6>>2] = $5;
 $7 = HEAP32[$0>>2]|0;
 $8 = (_glGetAttribLocation(($7|0),(7481|0))|0);
 $9 = ((($0)) + 12|0);
 HEAP32[$9>>2] = $8;
 $10 = HEAP32[$0>>2]|0;
 $11 = (_glGetAttribLocation(($10|0),(7497|0))|0);
 $12 = ((($0)) + 16|0);
 HEAP32[$12>>2] = $11;
 $13 = HEAP32[$0>>2]|0;
 $14 = (_glGetAttribLocation(($13|0),(7510|0))|0);
 $15 = ((($0)) + 20|0);
 HEAP32[$15>>2] = $14;
 $16 = HEAP32[$0>>2]|0;
 $17 = (_glGetAttribLocation(($16|0),(7524|0))|0);
 $18 = ((($0)) + 24|0);
 HEAP32[$18>>2] = $17;
 $19 = HEAP32[$0>>2]|0;
 $20 = (_glGetUniformLocation(($19|0),(7536|0))|0);
 $21 = ((($0)) + 28|0);
 HEAP32[$21>>2] = $20;
 $22 = HEAP32[$0>>2]|0;
 $23 = (_glGetUniformLocation(($22|0),(7546|0))|0);
 $24 = ((($0)) + 32|0);
 HEAP32[$24>>2] = $23;
 $25 = HEAP32[$0>>2]|0;
 $26 = (_glGetUniformLocation(($25|0),(7557|0))|0);
 $27 = ((($0)) + 36|0);
 HEAP32[$27>>2] = $26;
 $28 = HEAP32[$0>>2]|0;
 $29 = (_glGetUniformLocation(($28|0),(7568|0))|0);
 $30 = ((($0)) + 40|0);
 HEAP32[$30>>2] = $29;
 $31 = HEAP32[$0>>2]|0;
 $32 = (_glGetUniformLocation(($31|0),(7580|0))|0);
 $33 = ((($0)) + 44|0);
 HEAP32[$33>>2] = $32;
 $34 = HEAP32[$0>>2]|0;
 $35 = (_glGetUniformLocation(($34|0),(7589|0))|0);
 $36 = ((($0)) + 48|0);
 HEAP32[$36>>2] = $35;
 $37 = HEAP32[$0>>2]|0;
 $38 = (_glGetUniformLocation(($37|0),(7598|0))|0);
 $39 = ((($0)) + 52|0);
 HEAP32[$39>>2] = $38;
 return;
}
function _IsMouseButtonPressed($0) {
 $0 = $0|0;
 var $$0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = (16933 + ($0)|0);
 $2 = HEAP8[$1>>0]|0;
 $3 = (16936 + ($0)|0);
 $4 = HEAP8[$3>>0]|0;
 $5 = ($2<<24>>24)!=($4<<24>>24);
 $6 = ($2<<24>>24)==(1);
 $or$cond = $6 & $5;
 $$0 = $or$cond&1;
 return ($$0|0);
}
function _IsMouseButtonReleased($0) {
 $0 = $0|0;
 var $$0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = (16933 + ($0)|0);
 $2 = HEAP8[$1>>0]|0;
 $3 = (16936 + ($0)|0);
 $4 = HEAP8[$3>>0]|0;
 $5 = ($2<<24>>24)!=($4<<24>>24);
 $6 = ($2<<24>>24)==(0);
 $or$cond = $6 & $5;
 $$0 = $or$cond&1;
 return ($$0|0);
}
function _rlClearScreenBuffers() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 _glClear(16640);
 return;
}
function _CloseWindow() {
 var $0 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 _UnloadDefaultFont();
 _rlglClose();
 $0 = HEAP32[3530]|0;
 _glfwDestroyWindow(($0|0));
 _glfwTerminate();
 _TraceLog(0,8170,$vararg_buffer);
 STACKTOP = sp;return;
}
function _UnloadDefaultFont() {
 var $$byval_copy = 0, $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $$byval_copy = sp;
 ;HEAP32[$$byval_copy>>2]=HEAP32[14160>>2]|0;HEAP32[$$byval_copy+4>>2]=HEAP32[14160+4>>2]|0;HEAP32[$$byval_copy+8>>2]=HEAP32[14160+8>>2]|0;HEAP32[$$byval_copy+12>>2]=HEAP32[14160+12>>2]|0;HEAP32[$$byval_copy+16>>2]=HEAP32[14160+16>>2]|0;
 _UnloadTexture($$byval_copy);
 $0 = HEAP32[(14188)>>2]|0;
 _free($0);
 STACKTOP = sp;return;
}
function _rlglClose() {
 var $0 = 0, $1 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 _UnloadDefaultShader();
 _UnloadDefaultBuffers();
 _glDeleteTextures(1,(14472|0));
 $0 = HEAP32[3618]|0;
 HEAP32[$vararg_buffer>>2] = $0;
 _TraceLog(0,8197,$vararg_buffer);
 $1 = HEAP32[3648]|0;
 _free($1);
 STACKTOP = sp;return;
}
function _UnloadDefaultShader() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 _glUseProgram(0);
 $0 = HEAP32[3619]|0;
 _glDeleteProgram(($0|0));
 return;
}
function _UnloadDefaultBuffers() {
 var $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3611]|0;
 $1 = ($0|0)==(0);
 if (!($1)) {
  $2 = HEAP32[3613]|0;
  FUNCTION_TABLE_vi[$2 & 31](0);
 }
 _glDisableVertexAttribArray(0);
 _glDisableVertexAttribArray(1);
 _glDisableVertexAttribArray(2);
 _glDisableVertexAttribArray(3);
 _glBindBuffer(34962,0);
 _glBindBuffer(34963,0);
 _glDeleteBuffers(1,((15668)|0));
 _glDeleteBuffers(1,((15672)|0));
 _glDeleteBuffers(1,((15716)|0));
 _glDeleteBuffers(1,((15720)|0));
 _glDeleteBuffers(1,((15764)|0));
 _glDeleteBuffers(1,((15768)|0));
 _glDeleteBuffers(1,((15772)|0));
 _glDeleteBuffers(1,((15776)|0));
 $3 = HEAP32[3611]|0;
 $4 = ($3|0)==(0);
 if (!($4)) {
  $5 = HEAP32[3614]|0;
  FUNCTION_TABLE_vii[$5 & 63](1,(15664));
  $6 = HEAP32[3614]|0;
  FUNCTION_TABLE_vii[$6 & 63](1,(15712));
  $7 = HEAP32[3614]|0;
  FUNCTION_TABLE_vii[$7 & 63](1,(15760));
 }
 $8 = HEAP32[(15648)>>2]|0;
 _free($8);
 $9 = HEAP32[(15656)>>2]|0;
 _free($9);
 $10 = HEAP32[(15696)>>2]|0;
 _free($10);
 $11 = HEAP32[(15704)>>2]|0;
 _free($11);
 $12 = HEAP32[(15744)>>2]|0;
 _free($12);
 $13 = HEAP32[(15748)>>2]|0;
 _free($13);
 $14 = HEAP32[(15752)>>2]|0;
 _free($14);
 $15 = HEAP32[(15756)>>2]|0;
 _free($15);
 return;
}
function _UnloadTexture($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 $1 = HEAP32[$0>>2]|0;
 $2 = ($1|0)==(0);
 if ($2) {
  STACKTOP = sp;return;
 }
 _rlDeleteTextures($1);
 $3 = HEAP32[$0>>2]|0;
 HEAP32[$vararg_buffer>>2] = $3;
 _TraceLog(0,8262,$vararg_buffer);
 STACKTOP = sp;return;
}
function _rlDeleteTextures($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $1 = sp;
 HEAP32[$1>>2] = $0;
 $2 = ($0|0)==(0);
 if (!($2)) {
  _glDeleteTextures(1,($1|0));
 }
 STACKTOP = sp;return;
}
function _BeginDrawing() {
 var $0 = 0.0, $1 = 0.0, $2 = 0.0, $downscaleView$byval_copy = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $downscaleView$byval_copy = sp;
 $0 = (+_GetTime());
 HEAPF64[1756] = $0;
 $1 = +HEAPF64[1739];
 $2 = $0 - $1;
 HEAPF64[1757] = $2;
 HEAPF64[1739] = $0;
 _rlClearScreenBuffers();
 _rlLoadIdentity();
 dest=$downscaleView$byval_copy; src=14216; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 (_MatrixToFloat($downscaleView$byval_copy)|0);
 _rlMultMatrixf(15788);
 STACKTOP = sp;return;
}
function _MatrixToFloat($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0;
 var $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = HEAP32[$0>>2]|0;
 HEAP32[3947] = $1;
 $2 = ((($0)) + 4|0);
 $3 = HEAP32[$2>>2]|0;
 HEAP32[(15792)>>2] = $3;
 $4 = ((($0)) + 8|0);
 $5 = HEAP32[$4>>2]|0;
 HEAP32[(15796)>>2] = $5;
 $6 = ((($0)) + 12|0);
 $7 = HEAP32[$6>>2]|0;
 HEAP32[(15800)>>2] = $7;
 $8 = ((($0)) + 16|0);
 $9 = HEAP32[$8>>2]|0;
 HEAP32[(15804)>>2] = $9;
 $10 = ((($0)) + 20|0);
 $11 = HEAP32[$10>>2]|0;
 HEAP32[(15808)>>2] = $11;
 $12 = ((($0)) + 24|0);
 $13 = HEAP32[$12>>2]|0;
 HEAP32[(15812)>>2] = $13;
 $14 = ((($0)) + 28|0);
 $15 = HEAP32[$14>>2]|0;
 HEAP32[(15816)>>2] = $15;
 $16 = ((($0)) + 32|0);
 $17 = HEAP32[$16>>2]|0;
 HEAP32[(15820)>>2] = $17;
 $18 = ((($0)) + 36|0);
 $19 = HEAP32[$18>>2]|0;
 HEAP32[(15824)>>2] = $19;
 $20 = ((($0)) + 40|0);
 $21 = HEAP32[$20>>2]|0;
 HEAP32[(15828)>>2] = $21;
 $22 = ((($0)) + 44|0);
 $23 = HEAP32[$22>>2]|0;
 HEAP32[(15832)>>2] = $23;
 $24 = ((($0)) + 48|0);
 $25 = HEAP32[$24>>2]|0;
 HEAP32[(15836)>>2] = $25;
 $26 = ((($0)) + 52|0);
 $27 = HEAP32[$26>>2]|0;
 HEAP32[(15840)>>2] = $27;
 $28 = ((($0)) + 56|0);
 $29 = HEAP32[$28>>2]|0;
 HEAP32[(15844)>>2] = $29;
 $30 = ((($0)) + 60|0);
 $31 = HEAP32[$30>>2]|0;
 HEAP32[(15848)>>2] = $31;
 return (15788|0);
}
function _rlMultMatrixf($0) {
 $0 = $0|0;
 var $$byval_copy = 0, $$byval_copy1 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0;
 var $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0;
 var $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 256|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(256|0);
 $$byval_copy1 = sp + 192|0;
 $$byval_copy = sp + 128|0;
 $1 = sp + 64|0;
 $2 = sp;
 $3 = HEAP32[$0>>2]|0;
 HEAP32[$1>>2] = $3;
 $4 = ((($1)) + 4|0);
 $5 = ((($0)) + 4|0);
 $6 = HEAP32[$5>>2]|0;
 HEAP32[$4>>2] = $6;
 $7 = ((($1)) + 8|0);
 $8 = ((($0)) + 8|0);
 $9 = HEAP32[$8>>2]|0;
 HEAP32[$7>>2] = $9;
 $10 = ((($1)) + 12|0);
 $11 = ((($0)) + 12|0);
 $12 = HEAP32[$11>>2]|0;
 HEAP32[$10>>2] = $12;
 $13 = ((($1)) + 16|0);
 $14 = ((($0)) + 16|0);
 $15 = HEAP32[$14>>2]|0;
 HEAP32[$13>>2] = $15;
 $16 = ((($1)) + 20|0);
 $17 = ((($0)) + 20|0);
 $18 = HEAP32[$17>>2]|0;
 HEAP32[$16>>2] = $18;
 $19 = ((($1)) + 24|0);
 $20 = ((($0)) + 24|0);
 $21 = HEAP32[$20>>2]|0;
 HEAP32[$19>>2] = $21;
 $22 = ((($1)) + 28|0);
 $23 = ((($0)) + 28|0);
 $24 = HEAP32[$23>>2]|0;
 HEAP32[$22>>2] = $24;
 $25 = ((($1)) + 32|0);
 $26 = ((($0)) + 32|0);
 $27 = HEAP32[$26>>2]|0;
 HEAP32[$25>>2] = $27;
 $28 = ((($1)) + 36|0);
 $29 = ((($0)) + 36|0);
 $30 = HEAP32[$29>>2]|0;
 HEAP32[$28>>2] = $30;
 $31 = ((($1)) + 40|0);
 $32 = ((($0)) + 40|0);
 $33 = HEAP32[$32>>2]|0;
 HEAP32[$31>>2] = $33;
 $34 = ((($1)) + 44|0);
 $35 = ((($0)) + 44|0);
 $36 = HEAP32[$35>>2]|0;
 HEAP32[$34>>2] = $36;
 $37 = ((($1)) + 48|0);
 $38 = ((($0)) + 48|0);
 $39 = HEAP32[$38>>2]|0;
 HEAP32[$37>>2] = $39;
 $40 = ((($1)) + 52|0);
 $41 = ((($0)) + 52|0);
 $42 = HEAP32[$41>>2]|0;
 HEAP32[$40>>2] = $42;
 $43 = ((($1)) + 56|0);
 $44 = ((($0)) + 56|0);
 $45 = HEAP32[$44>>2]|0;
 HEAP32[$43>>2] = $45;
 $46 = ((($1)) + 60|0);
 $47 = ((($0)) + 60|0);
 $48 = HEAP32[$47>>2]|0;
 HEAP32[$46>>2] = $48;
 $49 = HEAP32[3577]|0;
 dest=$$byval_copy; src=$49; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy1; src=$1; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixMultiply($2,$$byval_copy,$$byval_copy1);
 dest=$49; src=$2; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _EndDrawing() {
 var $0 = 0.0, $1 = 0.0, $10 = 0.0, $11 = 0.0, $12 = 0.0, $13 = 0.0, $14 = 0.0, $2 = 0.0, $3 = 0.0, $4 = 0.0, $5 = 0.0, $6 = 0, $7 = 0.0, $8 = 0.0, $9 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 _rlglDraw();
 _SwapBuffers();
 _PollInputEvents();
 $0 = (+_GetTime());
 HEAPF64[1756] = $0;
 $1 = +HEAPF64[1739];
 $2 = $0 - $1;
 HEAPF64[1758] = $2;
 HEAPF64[1739] = $0;
 $3 = +HEAPF64[1757];
 $4 = $2 + $3;
 HEAPF64[1759] = $4;
 $5 = +HEAPF64[1736];
 $6 = $4 < $5;
 if (!($6)) {
  return;
 }
 $7 = $5 - $4;
 $8 = $7 * 1000.0;
 $9 = $8;
 _Wait($9);
 $10 = (+_GetTime());
 HEAPF64[1756] = $10;
 $11 = +HEAPF64[1739];
 $12 = $10 - $11;
 HEAPF64[1739] = $10;
 $13 = +HEAPF64[1759];
 $14 = $12 + $13;
 HEAPF64[1759] = $14;
 return;
}
function _rlglDraw() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 _UpdateDefaultBuffers();
 _DrawDefaultBuffers();
 return;
}
function _SwapBuffers() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3530]|0;
 _glfwSwapBuffers(($0|0));
 return;
}
function _PollInputEvents() {
 var $$04857 = 0, $$05160 = 0, $$058 = 0, $$lcssa = 0, $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0;
 var $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0.0, $31 = 0.0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0.0, $40 = 0;
 var $5 = 0.0, $6 = 0.0, $7 = 0.0, $8 = 0, $9 = 0, $scevgep = 0, $scevgep67 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 1456|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(1456|0);
 $0 = sp + 1440|0;
 $1 = sp + 1432|0;
 $2 = sp;
 _UpdateGestures();
 HEAP32[744] = -1;
 HEAP32[746] = -1;
 HEAP32[3963] = 0;
 $3 = HEAP32[3530]|0;
 _glfwGetCursorPos(($3|0),($0|0),($1|0));
 $4 = +HEAPF64[$0>>3];
 $5 = $4;
 HEAPF32[3470] = $5;
 $6 = +HEAPF64[$1>>3];
 $7 = $6;
 HEAPF32[(13884)>>2] = $7;
 _memcpy((17451|0),(16939|0),512)|0;
 ;HEAP8[16936>>0]=HEAP8[16933>>0]|0;HEAP8[16936+1>>0]=HEAP8[16933+1>>0]|0;HEAP8[16936+2>>0]=HEAP8[16933+2>>0]|0;
 $8 = HEAP32[3946]|0;
 HEAP32[3533] = $8;
 HEAP32[3946] = 0;
 $9 = (_emscripten_get_num_gamepads()|0);
 $10 = ($9|0)>(0);
 if (!($10)) {
  STACKTOP = sp;return;
 }
 $11 = ((($2)) + 12|0);
 $12 = ((($2)) + 8|0);
 $$05160 = 0;
 while(1) {
  $scevgep = (17963 + ($$05160<<5)|0);
  $scevgep67 = (18091 + ($$05160<<5)|0);
  dest=$scevgep; src=$scevgep67; stop=dest+32|0; do { HEAP8[dest>>0]=HEAP8[src>>0]|0; dest=dest+1|0; src=src+1|0; } while ((dest|0) < (stop|0));
  $13 = (_emscripten_get_gamepad_status(($$05160|0),($2|0))|0);
  $14 = ($13|0)==(0);
  if ($14) {
   $15 = HEAP32[$11>>2]|0;
   $16 = ($15|0)>(0);
   if ($16) {
    $17 = HEAP32[$11>>2]|0;
    $$04857 = 0;
    while(1) {
     $21 = (((($2)) + 1040|0) + ($$04857<<2)|0);
     $22 = HEAP32[$21>>2]|0;
     $23 = ($22|0)==(1);
     $24 = ((18091 + ($$05160<<5)|0) + ($$04857)|0);
     if ($23) {
      HEAP8[$24>>0] = 1;
      HEAP32[746] = $$04857;
     } else {
      HEAP8[$24>>0] = 0;
     }
     $25 = (($$04857) + 1)|0;
     $26 = ($25|0)<($17|0);
     $27 = ($25|0)<(32);
     $28 = $27 & $26;
     if ($28) {
      $$04857 = $25;
     } else {
      break;
     }
    }
   }
   $18 = HEAP32[$12>>2]|0;
   $19 = ($18|0)>(0);
   if ($19) {
    $20 = HEAP32[$12>>2]|0;
    $$058 = 0;
    while(1) {
     $29 = (((($2)) + 16|0) + ($$058<<3)|0);
     $30 = +HEAPF64[$29>>3];
     $31 = $30;
     $32 = ((15856 + ($$05160<<5)|0) + ($$058<<2)|0);
     HEAPF32[$32>>2] = $31;
     $33 = (($$058) + 1)|0;
     $34 = ($33|0)<($20|0);
     $35 = ($33|0)<(8);
     $36 = $35 & $34;
     if ($36) {
      $$058 = $33;
     } else {
      $$lcssa = $20;
      break;
     }
    }
   } else {
    $$lcssa = $18;
   }
   HEAP32[3963] = $$lcssa;
  }
  $37 = (($$05160) + 1)|0;
  $38 = ($37|0)<($9|0);
  $39 = ($37|0)<(4);
  $40 = $38 & $39;
  if ($40) {
   $$05160 = $37;
  } else {
   break;
  }
 }
 STACKTOP = sp;return;
}
function _Wait($0) {
 $0 = +$0;
 var $1 = 0.0, $2 = 0.0, $3 = 0.0, $4 = 0.0, $5 = 0, $6 = 0.0, $7 = 0.0, $8 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = (+_GetTime());
 $2 = 0.0 - $1;
 $3 = $0 / 1000.0;
 $4 = $3;
 $5 = $2 < $4;
 if (!($5)) {
  return;
 }
 while(1) {
  $6 = (+_GetTime());
  $7 = $6 - $1;
  $8 = $7 < $4;
  if (!($8)) {
   break;
  }
 }
 return;
}
function _UpdateDefaultBuffers() {
 var $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0;
 var $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0;
 var $45 = 0, $46 = 0, $47 = 0, $48 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3909]|0;
 $1 = ($0|0)>(0);
 if ($1) {
  $2 = HEAP32[3611]|0;
  $3 = ($2|0)==(0);
  if (!($3)) {
   $4 = HEAP32[3613]|0;
   $5 = HEAP32[(15664)>>2]|0;
   FUNCTION_TABLE_vi[$4 & 31]($5);
  }
  $6 = HEAP32[(15668)>>2]|0;
  _glBindBuffer(34962,($6|0));
  $7 = HEAP32[3909]|0;
  $8 = ($7*12)|0;
  $9 = HEAP32[(15648)>>2]|0;
  _glBufferSubData(34962,0,($8|0),($9|0));
  $10 = HEAP32[(15672)>>2]|0;
  _glBindBuffer(34962,($10|0));
  $11 = HEAP32[(15644)>>2]|0;
  $12 = $11 << 2;
  $13 = HEAP32[(15656)>>2]|0;
  _glBufferSubData(34962,0,($12|0),($13|0));
 }
 $14 = HEAP32[3921]|0;
 $15 = ($14|0)>(0);
 if ($15) {
  $16 = HEAP32[3611]|0;
  $17 = ($16|0)==(0);
  if (!($17)) {
   $18 = HEAP32[3613]|0;
   $19 = HEAP32[(15712)>>2]|0;
   FUNCTION_TABLE_vi[$18 & 31]($19);
  }
  $20 = HEAP32[(15716)>>2]|0;
  _glBindBuffer(34962,($20|0));
  $21 = HEAP32[3921]|0;
  $22 = ($21*12)|0;
  $23 = HEAP32[(15696)>>2]|0;
  _glBufferSubData(34962,0,($22|0),($23|0));
  $24 = HEAP32[(15720)>>2]|0;
  _glBindBuffer(34962,($24|0));
  $25 = HEAP32[(15692)>>2]|0;
  $26 = $25 << 2;
  $27 = HEAP32[(15704)>>2]|0;
  _glBufferSubData(34962,0,($26|0),($27|0));
 }
 $28 = HEAP32[3933]|0;
 $29 = ($28|0)>(0);
 if ($29) {
  $30 = HEAP32[3611]|0;
  $31 = ($30|0)==(0);
  if (!($31)) {
   $32 = HEAP32[3613]|0;
   $33 = HEAP32[(15760)>>2]|0;
   FUNCTION_TABLE_vi[$32 & 31]($33);
  }
  $34 = HEAP32[(15764)>>2]|0;
  _glBindBuffer(34962,($34|0));
  $35 = HEAP32[3933]|0;
  $36 = ($35*12)|0;
  $37 = HEAP32[(15744)>>2]|0;
  _glBufferSubData(34962,0,($36|0),($37|0));
  $38 = HEAP32[(15768)>>2]|0;
  _glBindBuffer(34962,($38|0));
  $39 = HEAP32[3933]|0;
  $40 = $39 << 3;
  $41 = HEAP32[(15748)>>2]|0;
  _glBufferSubData(34962,0,($40|0),($41|0));
  $42 = HEAP32[(15772)>>2]|0;
  _glBindBuffer(34962,($42|0));
  $43 = HEAP32[3933]|0;
  $44 = $43 << 2;
  $45 = HEAP32[(15752)>>2]|0;
  _glBufferSubData(34962,0,($44|0),($45|0));
 }
 $46 = HEAP32[3611]|0;
 $47 = ($46|0)==(0);
 if ($47) {
  return;
 }
 $48 = HEAP32[3613]|0;
 FUNCTION_TABLE_vi[$48 & 31](0);
 return;
}
function _DrawDefaultBuffers() {
 var $$ = 0, $$02830 = 0, $$02932 = 0, $$031 = 0, $$byval_copy2 = 0, $0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0;
 var $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0;
 var $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0;
 var $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0;
 var $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $modelview$byval_copy = 0;
 var $or$cond = 0, $or$cond3 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 320|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(320|0);
 $$byval_copy2 = sp + 256|0;
 $modelview$byval_copy = sp + 192|0;
 $0 = sp + 128|0;
 $1 = sp + 64|0;
 $2 = sp;
 dest=$0; src=14312; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$1; src=14376; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 $3 = HEAP32[3996]|0;
 $4 = ($3|0)!=(0);
 $$ = $4 ? 2 : 1;
 $$02932 = 0;
 while(1) {
  if ($4) {
   dest=$modelview$byval_copy; src=$0; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
   dest=$$byval_copy2; src=$1; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
   _SetStereoView($$02932,$modelview$byval_copy,$$byval_copy2);
  }
  $8 = HEAP32[3909]|0;
  $9 = ($8|0)>(0);
  $10 = HEAP32[3921]|0;
  $11 = ($10|0)>(0);
  $or$cond = $9 | $11;
  $12 = HEAP32[3933]|0;
  $13 = ($12|0)>(0);
  $or$cond3 = $or$cond | $13;
  if ($or$cond3) {
   $14 = HEAP32[3633]|0;
   _glUseProgram(($14|0));
   dest=$modelview$byval_copy; src=14376; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
   dest=$$byval_copy2; src=14312; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
   _MatrixMultiply($2,$modelview$byval_copy,$$byval_copy2);
   $15 = HEAP32[(14560)>>2]|0;
   dest=$$byval_copy2; src=$2; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
   $16 = (_MatrixToFloat($$byval_copy2)|0);
   _glUniformMatrix4fv(($15|0),1,0,($16|0));
   $17 = HEAP32[(14564)>>2]|0;
   _glUniform4f(($17|0),1.0,1.0,1.0,1.0);
   $18 = HEAP32[(14576)>>2]|0;
   _glUniform1i(($18|0),0);
  }
  $19 = HEAP32[3909]|0;
  $20 = ($19|0)>(0);
  if ($20) {
   $21 = HEAP32[3618]|0;
   _glBindTexture(3553,($21|0));
   $22 = HEAP32[3611]|0;
   $23 = ($22|0)==(0);
   if ($23) {
    $26 = HEAP32[(15668)>>2]|0;
    _glBindBuffer(34962,($26|0));
    $27 = HEAP32[(14536)>>2]|0;
    _glVertexAttribPointer(($27|0),3,5126,0,0,(0|0));
    $28 = HEAP32[(14536)>>2]|0;
    _glEnableVertexAttribArray(($28|0));
    $29 = HEAP32[(15672)>>2]|0;
    _glBindBuffer(34962,($29|0));
    $30 = HEAP32[(14556)>>2]|0;
    _glVertexAttribPointer(($30|0),4,5121,1,0,(0|0));
    $31 = HEAP32[(14556)>>2]|0;
    _glEnableVertexAttribArray(($31|0));
   } else {
    $24 = HEAP32[3613]|0;
    $25 = HEAP32[(15664)>>2]|0;
    FUNCTION_TABLE_vi[$24 & 31]($25);
   }
   $32 = HEAP32[3909]|0;
   _glDrawArrays(1,0,($32|0));
   $33 = HEAP32[3611]|0;
   $34 = ($33|0)==(0);
   if ($34) {
    _glBindBuffer(34962,0);
   }
   _glBindTexture(3553,0);
  }
  $35 = HEAP32[3921]|0;
  $36 = ($35|0)>(0);
  if ($36) {
   $37 = HEAP32[3618]|0;
   _glBindTexture(3553,($37|0));
   $38 = HEAP32[3611]|0;
   $39 = ($38|0)==(0);
   if ($39) {
    $42 = HEAP32[(15716)>>2]|0;
    _glBindBuffer(34962,($42|0));
    $43 = HEAP32[(14536)>>2]|0;
    _glVertexAttribPointer(($43|0),3,5126,0,0,(0|0));
    $44 = HEAP32[(14536)>>2]|0;
    _glEnableVertexAttribArray(($44|0));
    $45 = HEAP32[(15720)>>2]|0;
    _glBindBuffer(34962,($45|0));
    $46 = HEAP32[(14556)>>2]|0;
    _glVertexAttribPointer(($46|0),4,5121,1,0,(0|0));
    $47 = HEAP32[(14556)>>2]|0;
    _glEnableVertexAttribArray(($47|0));
   } else {
    $40 = HEAP32[3613]|0;
    $41 = HEAP32[(15712)>>2]|0;
    FUNCTION_TABLE_vi[$40 & 31]($41);
   }
   $48 = HEAP32[3921]|0;
   _glDrawArrays(4,0,($48|0));
   $49 = HEAP32[3611]|0;
   $50 = ($49|0)==(0);
   if ($50) {
    _glBindBuffer(34962,0);
   }
   _glBindTexture(3553,0);
  }
  $51 = HEAP32[3933]|0;
  $52 = ($51|0)>(0);
  if ($52) {
   $53 = HEAP32[3611]|0;
   $54 = ($53|0)==(0);
   if ($54) {
    $57 = HEAP32[(15764)>>2]|0;
    _glBindBuffer(34962,($57|0));
    $58 = HEAP32[(14536)>>2]|0;
    _glVertexAttribPointer(($58|0),3,5126,0,0,(0|0));
    $59 = HEAP32[(14536)>>2]|0;
    _glEnableVertexAttribArray(($59|0));
    $60 = HEAP32[(15768)>>2]|0;
    _glBindBuffer(34962,($60|0));
    $61 = HEAP32[(14540)>>2]|0;
    _glVertexAttribPointer(($61|0),2,5126,0,0,(0|0));
    $62 = HEAP32[(14540)>>2]|0;
    _glEnableVertexAttribArray(($62|0));
    $63 = HEAP32[(15772)>>2]|0;
    _glBindBuffer(34962,($63|0));
    $64 = HEAP32[(14556)>>2]|0;
    _glVertexAttribPointer(($64|0),4,5121,1,0,(0|0));
    $65 = HEAP32[(14556)>>2]|0;
    _glEnableVertexAttribArray(($65|0));
    $66 = HEAP32[(15776)>>2]|0;
    _glBindBuffer(34963,($66|0));
   } else {
    $55 = HEAP32[3613]|0;
    $56 = HEAP32[(15760)>>2]|0;
    FUNCTION_TABLE_vi[$55 & 31]($56);
   }
   $67 = HEAP32[3649]|0;
   $68 = ($67|0)>(0);
   if ($68) {
    $$02830 = 0;$$031 = 0;
    while(1) {
     $71 = HEAP32[3648]|0;
     $72 = (($71) + (($$031*144)|0)|0);
     $73 = HEAP32[$72>>2]|0;
     $74 = (($73|0) / 4)&-1;
     $75 = ($74*6)|0;
     $76 = (((($71) + (($$031*144)|0)|0)) + 8|0);
     $77 = HEAP32[$76>>2]|0;
     _glBindTexture(3553,($77|0));
     $78 = $$02830 << 1;
     $79 = $78;
     _glDrawElements(4,($75|0),5123,($79|0));
     $80 = HEAP32[3648]|0;
     $81 = (($80) + (($$031*144)|0)|0);
     $82 = HEAP32[$81>>2]|0;
     $83 = (($82|0) / 4)&-1;
     $84 = ($83*6)|0;
     $85 = (($84) + ($$02830))|0;
     $86 = (($$031) + 1)|0;
     $87 = HEAP32[3649]|0;
     $88 = ($86|0)<($87|0);
     if ($88) {
      $$02830 = $85;$$031 = $86;
     } else {
      break;
     }
    }
   }
   $69 = HEAP32[3611]|0;
   $70 = ($69|0)==(0);
   if ($70) {
    _glBindBuffer(34962,0);
    _glBindBuffer(34963,0);
   }
   _glBindTexture(3553,0);
  }
  $89 = HEAP32[3611]|0;
  $90 = ($89|0)==(0);
  if (!($90)) {
   $91 = HEAP32[3613]|0;
   FUNCTION_TABLE_vi[$91 & 31](0);
  }
  _glUseProgram(0);
  $92 = (($$02932) + 1)|0;
  $93 = ($92|0)<($$|0);
  if ($93) {
   $$02932 = $92;
  } else {
   break;
  }
 }
 HEAP32[3649] = 1;
 $5 = HEAP32[3618]|0;
 $6 = HEAP32[3648]|0;
 $7 = ((($6)) + 8|0);
 HEAP32[$7>>2] = $5;
 HEAP32[$6>>2] = 0;
 HEAP32[3909] = 0;
 HEAP32[(15644)>>2] = 0;
 HEAP32[3921] = 0;
 HEAP32[(15692)>>2] = 0;
 HEAP32[3933] = 0;
 HEAP32[(15736)>>2] = 0;
 HEAP32[(15740)>>2] = 0;
 HEAPF32[747] = -1.0;
 dest=14312; src=$0; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=14376; src=$1; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _SetStereoView($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$byval_copy = 0, $$byval_copy3 = 0, $10 = 0, $11 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 256|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(256|0);
 $$byval_copy3 = sp + 192|0;
 $$byval_copy = sp + 64|0;
 $3 = sp;
 $4 = sp + 128|0;
 dest=$3; src=$1; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 $5 = HEAP32[3907]|0;
 $6 = Math_imul($5, $0)|0;
 $7 = (($6|0) / 2)&-1;
 $8 = (($5|0) / 2)&-1;
 $9 = HEAP32[3908]|0;
 _rlViewport($7,0,$8,$9);
 $10 = (16216 + ($0<<6)|0);
 dest=$$byval_copy; src=$2; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy3; src=$10; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixMultiply($4,$$byval_copy,$$byval_copy3);
 $11 = (16088 + ($0<<6)|0);
 dest=$3; src=$11; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy3; src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _SetMatrixModelview($$byval_copy3);
 dest=$$byval_copy3; src=$3; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _SetMatrixProjection($$byval_copy3);
 STACKTOP = sp;return;
}
function _SetMatrixModelview($0) {
 $0 = $0|0;
 var dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 dest=14376; src=$0; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 return;
}
function _SetMatrixProjection($0) {
 $0 = $0|0;
 var dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 dest=14312; src=$0; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 return;
}
function _rlPushMatrix() {
 var $0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $vararg_buffer = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 $0 = HEAP32[4086]|0;
 $1 = ($0|0)==(15);
 if ($1) {
  HEAP32[$vararg_buffer>>2] = 16;
  _TraceLog(1,8312,$vararg_buffer);
 }
 $2 = HEAP32[4086]|0;
 $3 = (14604 + ($2<<6)|0);
 $4 = HEAP32[3577]|0;
 dest=$3; src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _rlLoadIdentity();
 $5 = HEAP32[4086]|0;
 $6 = (($5) + 1)|0;
 HEAP32[4086] = $6;
 $7 = HEAP32[3610]|0;
 $8 = ($7|0)==(5888);
 if (!($8)) {
  STACKTOP = sp;return;
 }
 HEAP32[4087] = 1;
 STACKTOP = sp;return;
}
function _rlPopMatrix() {
 var $0 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[4086]|0;
 $1 = ($0|0)>(0);
 if (!($1)) {
  return;
 }
 $2 = HEAP32[4086]|0;
 $3 = (($2) + -1)|0;
 $4 = (14604 + ($3<<6)|0);
 $5 = HEAP32[3577]|0;
 _memmove(($5|0),($4|0),64)|0;
 $6 = (($2) + -1)|0;
 HEAP32[4086] = $6;
 return;
}
function _GetFrameTime() {
 var $0 = 0.0, $1 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = +HEAPF64[1759];
 $1 = $0;
 return (+$1);
}
function _rlTranslatef($0,$1,$2) {
 $0 = +$0;
 $1 = +$1;
 $2 = +$2;
 var $$byval_copy = 0, $$byval_copy1 = 0, $3 = 0, $4 = 0, $5 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 256|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(256|0);
 $$byval_copy1 = sp + 192|0;
 $$byval_copy = sp + 128|0;
 $3 = sp + 64|0;
 $4 = sp;
 _MatrixTranslate($3,$0,$1,$2);
 _MatrixTranspose($3);
 $5 = HEAP32[3577]|0;
 dest=$$byval_copy; src=$5; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy1; src=$3; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixMultiply($4,$$byval_copy,$$byval_copy1);
 dest=$5; src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _rlRotatef($0,$1,$2,$3) {
 $0 = +$0;
 $1 = +$1;
 $2 = +$2;
 $3 = +$3;
 var $$byval_copy1 = 0, $$byval_copy2 = 0, $10 = 0.0, $11 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 336|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(336|0);
 $$byval_copy2 = sp + 272|0;
 $$byval_copy1 = sp + 208|0;
 $4 = sp + 144|0;
 $5 = sp + 64|0;
 $6 = sp + 80|0;
 $7 = sp;
 _MatrixIdentity($4);
 HEAPF32[$5>>2] = $1;
 $8 = ((($5)) + 4|0);
 HEAPF32[$8>>2] = $2;
 $9 = ((($5)) + 8|0);
 HEAPF32[$9>>2] = $3;
 _VectorNormalize($5);
 $10 = $0 * 0.01745329238474369;
 ;HEAP32[$$byval_copy2>>2]=HEAP32[$5>>2]|0;HEAP32[$$byval_copy2+4>>2]=HEAP32[$5+4>>2]|0;HEAP32[$$byval_copy2+8>>2]=HEAP32[$5+8>>2]|0;
 _MatrixRotate($6,$$byval_copy2,$10);
 dest=$4; src=$6; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixTranspose($4);
 $11 = HEAP32[3577]|0;
 dest=$$byval_copy1; src=$11; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 dest=$$byval_copy2; src=$4; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 _MatrixMultiply($7,$$byval_copy1,$$byval_copy2);
 dest=$11; src=$7; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 STACKTOP = sp;return;
}
function _rlBegin($0) {
 $0 = $0|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 HEAP32[3650] = $0;
 return;
}
function _rlEnd() {
 var $$03956 = 0, $$04052 = 0, $$04154 = 0, $$04248 = 0, $$04347 = 0, $$byval_copy = 0, $$promoted = 0, $0 = 0, $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0;
 var $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0;
 var $128 = 0, $129 = 0, $13 = 0.0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0;
 var $146 = 0, $147 = 0, $148 = 0.0, $149 = 0.0, $15 = 0.0, $16 = 0, $17 = 0.0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0;
 var $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0;
 var $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0;
 var $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0;
 var $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $exitcond = 0, $exitcond60 = 0, $exitcond63 = 0;
 var $scevgep = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $$byval_copy = sp;
 $0 = HEAP32[4087]|0;
 $1 = ($0|0)==(0);
 if (!($1)) {
  $2 = HEAP32[4088]|0;
  $3 = ($2|0)>(0);
  if ($3) {
   $$03956 = 0;
   while(1) {
    $6 = HEAP32[3647]|0;
    $7 = (($6) + (($$03956*12)|0)|0);
    $8 = HEAP32[3577]|0;
    dest=$$byval_copy; src=$8; stop=dest+64|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
    _VectorTransform($7,$$byval_copy);
    $9 = (($$03956) + 1)|0;
    $5 = HEAP32[4088]|0;
    $10 = ($9|0)<($5|0);
    if ($10) {
     $$03956 = $9;
    } else {
     break;
    }
   }
   HEAP32[4087] = 0;
   $4 = ($5|0)>(0);
   if ($4) {
    $$04154 = 0;
    while(1) {
     $11 = HEAP32[3647]|0;
     $12 = (($11) + (($$04154*12)|0)|0);
     $13 = +HEAPF32[$12>>2];
     $14 = (((($11) + (($$04154*12)|0)|0)) + 4|0);
     $15 = +HEAPF32[$14>>2];
     $16 = (((($11) + (($$04154*12)|0)|0)) + 8|0);
     $17 = +HEAPF32[$16>>2];
     _rlVertex3f($13,$15,$17);
     $18 = (($$04154) + 1)|0;
     $19 = HEAP32[4088]|0;
     $20 = ($18|0)<($19|0);
     if ($20) {
      $$04154 = $18;
     } else {
      break;
     }
    }
   }
  } else {
   HEAP32[4087] = 0;
  }
  HEAP32[4088] = 0;
 }
 $21 = HEAP32[3650]|0;
 switch ($21|0) {
 case 1:  {
  $22 = HEAP32[3909]|0;
  $23 = HEAP32[(15644)>>2]|0;
  $24 = ($22|0)==($23|0);
  if ($24) {
   $148 = +HEAPF32[747];
   $149 = $148 + 4.9999998736893758E-5;
   HEAPF32[747] = $149;
   STACKTOP = sp;return;
  }
  $25 = (($22) - ($23))|0;
  $26 = ($25|0)>(0);
  if ($26) {
   $$04347 = 0;
  } else {
   $148 = +HEAPF32[747];
   $149 = $148 + 4.9999998736893758E-5;
   HEAPF32[747] = $149;
   STACKTOP = sp;return;
  }
  while(1) {
   $27 = HEAP32[(15656)>>2]|0;
   $28 = HEAP32[(15644)>>2]|0;
   $29 = $28 << 2;
   $30 = (($29) + -4)|0;
   $31 = (($27) + ($30)|0);
   $32 = HEAP8[$31>>0]|0;
   $33 = (($27) + ($29)|0);
   HEAP8[$33>>0] = $32;
   $34 = HEAP32[(15656)>>2]|0;
   $35 = HEAP32[(15644)>>2]|0;
   $36 = $35 << 2;
   $37 = (($36) + -3)|0;
   $38 = (($34) + ($37)|0);
   $39 = HEAP8[$38>>0]|0;
   $40 = $36 | 1;
   $41 = (($34) + ($40)|0);
   HEAP8[$41>>0] = $39;
   $42 = HEAP32[(15656)>>2]|0;
   $43 = HEAP32[(15644)>>2]|0;
   $44 = $43 << 2;
   $45 = (($44) + -2)|0;
   $46 = (($42) + ($45)|0);
   $47 = HEAP8[$46>>0]|0;
   $48 = $44 | 2;
   $49 = (($42) + ($48)|0);
   HEAP8[$49>>0] = $47;
   $50 = HEAP32[(15656)>>2]|0;
   $51 = HEAP32[(15644)>>2]|0;
   $52 = $51 << 2;
   $53 = (($52) + -1)|0;
   $54 = (($50) + ($53)|0);
   $55 = HEAP8[$54>>0]|0;
   $56 = $52 | 3;
   $57 = (($50) + ($56)|0);
   HEAP8[$57>>0] = $55;
   $58 = HEAP32[(15644)>>2]|0;
   $59 = (($58) + 1)|0;
   HEAP32[(15644)>>2] = $59;
   $60 = (($$04347) + 1)|0;
   $exitcond = ($60|0)==($25|0);
   if ($exitcond) {
    break;
   } else {
    $$04347 = $60;
   }
  }
  $148 = +HEAPF32[747];
  $149 = $148 + 4.9999998736893758E-5;
  HEAPF32[747] = $149;
  STACKTOP = sp;return;
  break;
 }
 case 4:  {
  $61 = HEAP32[3921]|0;
  $62 = HEAP32[(15692)>>2]|0;
  $63 = ($61|0)==($62|0);
  if ($63) {
   $148 = +HEAPF32[747];
   $149 = $148 + 4.9999998736893758E-5;
   HEAPF32[747] = $149;
   STACKTOP = sp;return;
  }
  $64 = (($61) - ($62))|0;
  $65 = ($64|0)>(0);
  if ($65) {
   $$04248 = 0;
  } else {
   $148 = +HEAPF32[747];
   $149 = $148 + 4.9999998736893758E-5;
   HEAPF32[747] = $149;
   STACKTOP = sp;return;
  }
  while(1) {
   $66 = HEAP32[(15704)>>2]|0;
   $67 = HEAP32[(15692)>>2]|0;
   $68 = $67 << 2;
   $69 = (($68) + -4)|0;
   $70 = (($66) + ($69)|0);
   $71 = HEAP8[$70>>0]|0;
   $72 = (($66) + ($68)|0);
   HEAP8[$72>>0] = $71;
   $73 = HEAP32[(15704)>>2]|0;
   $74 = HEAP32[(15692)>>2]|0;
   $75 = $74 << 2;
   $76 = (($75) + -3)|0;
   $77 = (($73) + ($76)|0);
   $78 = HEAP8[$77>>0]|0;
   $79 = $75 | 1;
   $80 = (($73) + ($79)|0);
   HEAP8[$80>>0] = $78;
   $81 = HEAP32[(15704)>>2]|0;
   $82 = HEAP32[(15692)>>2]|0;
   $83 = $82 << 2;
   $84 = (($83) + -2)|0;
   $85 = (($81) + ($84)|0);
   $86 = HEAP8[$85>>0]|0;
   $87 = $83 | 2;
   $88 = (($81) + ($87)|0);
   HEAP8[$88>>0] = $86;
   $89 = HEAP32[(15704)>>2]|0;
   $90 = HEAP32[(15692)>>2]|0;
   $91 = $90 << 2;
   $92 = (($91) + -1)|0;
   $93 = (($89) + ($92)|0);
   $94 = HEAP8[$93>>0]|0;
   $95 = $91 | 3;
   $96 = (($89) + ($95)|0);
   HEAP8[$96>>0] = $94;
   $97 = HEAP32[(15692)>>2]|0;
   $98 = (($97) + 1)|0;
   HEAP32[(15692)>>2] = $98;
   $99 = (($$04248) + 1)|0;
   $exitcond60 = ($99|0)==($64|0);
   if ($exitcond60) {
    break;
   } else {
    $$04248 = $99;
   }
  }
  $148 = +HEAPF32[747];
  $149 = $148 + 4.9999998736893758E-5;
  HEAPF32[747] = $149;
  STACKTOP = sp;return;
  break;
 }
 case 7:  {
  $100 = HEAP32[3933]|0;
  $101 = HEAP32[(15740)>>2]|0;
  $102 = ($100|0)==($101|0);
  if (!($102)) {
   $103 = (($100) - ($101))|0;
   $104 = ($103|0)>(0);
   if ($104) {
    $$04052 = 0;
    while(1) {
     $105 = HEAP32[(15752)>>2]|0;
     $106 = HEAP32[(15740)>>2]|0;
     $107 = $106 << 2;
     $108 = (($107) + -4)|0;
     $109 = (($105) + ($108)|0);
     $110 = HEAP8[$109>>0]|0;
     $111 = (($105) + ($107)|0);
     HEAP8[$111>>0] = $110;
     $112 = HEAP32[(15752)>>2]|0;
     $113 = HEAP32[(15740)>>2]|0;
     $114 = $113 << 2;
     $115 = (($114) + -3)|0;
     $116 = (($112) + ($115)|0);
     $117 = HEAP8[$116>>0]|0;
     $118 = $114 | 1;
     $119 = (($112) + ($118)|0);
     HEAP8[$119>>0] = $117;
     $120 = HEAP32[(15752)>>2]|0;
     $121 = HEAP32[(15740)>>2]|0;
     $122 = $121 << 2;
     $123 = (($122) + -2)|0;
     $124 = (($120) + ($123)|0);
     $125 = HEAP8[$124>>0]|0;
     $126 = $122 | 2;
     $127 = (($120) + ($126)|0);
     HEAP8[$127>>0] = $125;
     $128 = HEAP32[(15752)>>2]|0;
     $129 = HEAP32[(15740)>>2]|0;
     $130 = $129 << 2;
     $131 = (($130) + -1)|0;
     $132 = (($128) + ($131)|0);
     $133 = HEAP8[$132>>0]|0;
     $134 = $130 | 3;
     $135 = (($128) + ($134)|0);
     HEAP8[$135>>0] = $133;
     $136 = HEAP32[(15740)>>2]|0;
     $137 = (($136) + 1)|0;
     HEAP32[(15740)>>2] = $137;
     $138 = (($$04052) + 1)|0;
     $exitcond63 = ($138|0)==($103|0);
     if ($exitcond63) {
      break;
     } else {
      $$04052 = $138;
     }
    }
   }
  }
  $139 = HEAP32[3933]|0;
  $140 = HEAP32[(15736)>>2]|0;
  $141 = ($139|0)>($140|0);
  if (!($141)) {
   $148 = +HEAPF32[747];
   $149 = $148 + 4.9999998736893758E-5;
   HEAPF32[747] = $149;
   STACKTOP = sp;return;
  }
  $142 = HEAP32[(15748)>>2]|0;
  $$promoted = HEAP32[(15736)>>2]|0;
  $143 = $$promoted << 1;
  $scevgep = (($142) + ($143<<2)|0);
  $144 = (($139) - ($140))|0;
  $145 = $144 << 3;
  _memset(($scevgep|0),0,($145|0))|0;
  $146 = (($139) + ($$promoted))|0;
  $147 = (($146) - ($140))|0;
  HEAP32[(15736)>>2] = $147;
  $148 = +HEAPF32[747];
  $149 = $148 + 4.9999998736893758E-5;
  HEAPF32[747] = $149;
  STACKTOP = sp;return;
  break;
 }
 default: {
  $148 = +HEAPF32[747];
  $149 = $148 + 4.9999998736893758E-5;
  HEAPF32[747] = $149;
  STACKTOP = sp;return;
 }
 }
}
function _rlVertex3f($0,$1,$2) {
 $0 = +$0;
 $1 = +$1;
 $2 = +$2;
 var $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0;
 var $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0;
 var $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer1 = 0, $vararg_buffer3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $vararg_buffer3 = sp + 16|0;
 $vararg_buffer1 = sp + 8|0;
 $vararg_buffer = sp;
 $3 = HEAP32[4087]|0;
 $4 = ($3|0)==(0);
 if (!($4)) {
  $5 = HEAP32[3647]|0;
  $6 = HEAP32[4088]|0;
  $7 = (($5) + (($6*12)|0)|0);
  HEAPF32[$7>>2] = $0;
  $8 = (((($5) + (($6*12)|0)|0)) + 4|0);
  HEAPF32[$8>>2] = $1;
  $9 = (((($5) + (($6*12)|0)|0)) + 8|0);
  HEAPF32[$9>>2] = $2;
  $10 = (($6) + 1)|0;
  HEAP32[4088] = $10;
  STACKTOP = sp;return;
 }
 $11 = HEAP32[3650]|0;
 switch ($11|0) {
 case 1:  {
  $12 = HEAP32[3909]|0;
  $13 = ($12|0)<(2048);
  if ($13) {
   $14 = HEAP32[(15648)>>2]|0;
   $15 = ($12*3)|0;
   $16 = (($14) + ($15<<2)|0);
   HEAPF32[$16>>2] = $0;
   $17 = (($15) + 1)|0;
   $18 = (($14) + ($17<<2)|0);
   HEAPF32[$18>>2] = $1;
   $19 = (($15) + 2)|0;
   $20 = (($14) + ($19<<2)|0);
   HEAPF32[$20>>2] = $2;
   $21 = (($12) + 1)|0;
   HEAP32[3909] = $21;
   STACKTOP = sp;return;
  } else {
   _TraceLog(1,8350,$vararg_buffer);
   STACKTOP = sp;return;
  }
  break;
 }
 case 4:  {
  $22 = HEAP32[3921]|0;
  $23 = ($22|0)<(6144);
  if ($23) {
   $24 = HEAP32[(15696)>>2]|0;
   $25 = ($22*3)|0;
   $26 = (($24) + ($25<<2)|0);
   HEAPF32[$26>>2] = $0;
   $27 = (($25) + 1)|0;
   $28 = (($24) + ($27<<2)|0);
   HEAPF32[$28>>2] = $1;
   $29 = (($25) + 2)|0;
   $30 = (($24) + ($29<<2)|0);
   HEAPF32[$30>>2] = $2;
   $31 = (($22) + 1)|0;
   HEAP32[3921] = $31;
   STACKTOP = sp;return;
  } else {
   _TraceLog(1,8375,$vararg_buffer1);
   STACKTOP = sp;return;
  }
  break;
 }
 case 7:  {
  $32 = HEAP32[3933]|0;
  $33 = ($32|0)<(4096);
  if ($33) {
   $34 = HEAP32[(15744)>>2]|0;
   $35 = ($32*3)|0;
   $36 = (($34) + ($35<<2)|0);
   HEAPF32[$36>>2] = $0;
   $37 = (($35) + 1)|0;
   $38 = (($34) + ($37<<2)|0);
   HEAPF32[$38>>2] = $1;
   $39 = (($35) + 2)|0;
   $40 = (($34) + ($39<<2)|0);
   HEAPF32[$40>>2] = $2;
   $41 = (($32) + 1)|0;
   HEAP32[3933] = $41;
   $42 = HEAP32[3648]|0;
   $43 = HEAP32[3649]|0;
   $44 = (($43) + -1)|0;
   $45 = (($42) + (($44*144)|0)|0);
   $46 = HEAP32[$45>>2]|0;
   $47 = (($46) + 1)|0;
   HEAP32[$45>>2] = $47;
   STACKTOP = sp;return;
  } else {
   _TraceLog(1,8404,$vararg_buffer3);
   STACKTOP = sp;return;
  }
  break;
 }
 default: {
  STACKTOP = sp;return;
 }
 }
}
function _rlVertex2f($0,$1) {
 $0 = +$0;
 $1 = +$1;
 var $2 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = +HEAPF32[747];
 _rlVertex3f($0,$1,$2);
 return;
}
function _rlTexCoord2f($0,$1) {
 $0 = +$0;
 $1 = +$1;
 var $10 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP32[3650]|0;
 $3 = ($2|0)==(7);
 if (!($3)) {
  return;
 }
 $4 = HEAP32[(15748)>>2]|0;
 $5 = HEAP32[(15736)>>2]|0;
 $6 = $5 << 1;
 $7 = (($4) + ($6<<2)|0);
 HEAPF32[$7>>2] = $0;
 $8 = $6 | 1;
 $9 = (($4) + ($8<<2)|0);
 HEAPF32[$9>>2] = $1;
 $10 = (($5) + 1)|0;
 HEAP32[(15736)>>2] = $10;
 return;
}
function _rlNormal3f($0,$1,$2) {
 $0 = +$0;
 $1 = +$1;
 $2 = +$2;
 var label = 0, sp = 0;
 sp = STACKTOP;
 return;
}
function _rlColor4ub($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $$sink37 = 0, $$sink38 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $4 = 0, $5 = 0;
 var $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $4 = HEAP32[3650]|0;
 switch ($4|0) {
 case 1:  {
  $$sink37 = (15644);$$sink38 = (15656);
  break;
 }
 case 4:  {
  $$sink37 = (15692);$$sink38 = (15704);
  break;
 }
 case 7:  {
  $$sink37 = (15740);$$sink38 = (15752);
  break;
 }
 default: {
  return;
 }
 }
 $5 = HEAP32[$$sink38>>2]|0;
 $6 = HEAP32[$$sink37>>2]|0;
 $7 = $6 << 2;
 $8 = (($5) + ($7)|0);
 HEAP8[$8>>0] = $0;
 $9 = HEAP32[$$sink38>>2]|0;
 $10 = HEAP32[$$sink37>>2]|0;
 $11 = $10 << 2;
 $12 = $11 | 1;
 $13 = (($9) + ($12)|0);
 HEAP8[$13>>0] = $1;
 $14 = HEAP32[$$sink38>>2]|0;
 $15 = HEAP32[$$sink37>>2]|0;
 $16 = $15 << 2;
 $17 = $16 | 2;
 $18 = (($14) + ($17)|0);
 HEAP8[$18>>0] = $2;
 $19 = HEAP32[$$sink38>>2]|0;
 $20 = HEAP32[$$sink37>>2]|0;
 $21 = $20 << 2;
 $22 = $21 | 3;
 $23 = (($19) + ($22)|0);
 HEAP8[$23>>0] = $3;
 $24 = HEAP32[$$sink37>>2]|0;
 $25 = (($24) + 1)|0;
 HEAP32[$$sink37>>2] = $25;
 return;
}
function _rlEnableTexture($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = HEAP32[3648]|0;
 $2 = HEAP32[3649]|0;
 $3 = (($2) + -1)|0;
 $4 = (((($1) + (($3*144)|0)|0)) + 8|0);
 $5 = HEAP32[$4>>2]|0;
 $6 = ($5|0)==($0|0);
 if ($6) {
  return;
 }
 $7 = (($1) + (($3*144)|0)|0);
 $8 = HEAP32[$7>>2]|0;
 $9 = ($8|0)>(0);
 if ($9) {
  $10 = (($2) + 1)|0;
  HEAP32[3649] = $10;
 }
 $11 = HEAP32[3649]|0;
 $12 = (($11) + -1)|0;
 $13 = (((($1) + (($12*144)|0)|0)) + 8|0);
 HEAP32[$13>>2] = $0;
 $14 = (($1) + (($12*144)|0)|0);
 HEAP32[$14>>2] = 0;
 return;
}
function _rlDisableTexture() {
 var $0 = 0, $1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = HEAP32[3933]|0;
 $1 = ($0|0)>(4095);
 if (!($1)) {
  return;
 }
 _rlglDraw();
 return;
}
function _GetDefaultFont($0) {
 $0 = $0|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 ;HEAP32[$0>>2]=HEAP32[14160>>2]|0;HEAP32[$0+4>>2]=HEAP32[14160+4>>2]|0;HEAP32[$0+8>>2]=HEAP32[14160+8>>2]|0;HEAP32[$0+12>>2]=HEAP32[14160+12>>2]|0;HEAP32[$0+16>>2]=HEAP32[14160+16>>2]|0;HEAP32[$0+20>>2]=HEAP32[14160+20>>2]|0;HEAP32[$0+24>>2]=HEAP32[14160+24>>2]|0;HEAP32[$0+28>>2]=HEAP32[14160+28>>2]|0;
 return;
}
function _GetCharIndex($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$08 = 0, $$09 = 0, $10 = 0, $11 = 0, $12 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ((($0)) + 24|0);
 $3 = HEAP32[$2>>2]|0;
 $4 = ($3|0)>(0);
 if (!($4)) {
  $$08 = 0;
  return ($$08|0);
 }
 $5 = ((($0)) + 28|0);
 $6 = HEAP32[$5>>2]|0;
 $$09 = 0;
 while(1) {
  $7 = (($6) + ($$09<<5)|0);
  $8 = HEAP32[$7>>2]|0;
  $9 = ($8|0)==($1|0);
  if ($9) {
   $$08 = $$09;
   label = 5;
   break;
  }
  $10 = (($$09) + 1)|0;
  $11 = HEAP32[$2>>2]|0;
  $12 = ($10|0)<($11|0);
  if ($12) {
   $$09 = $10;
  } else {
   $$08 = 0;
   label = 5;
   break;
  }
 }
 if ((label|0) == 5) {
  return ($$08|0);
 }
 return (0)|0;
}
function _DrawTexturePro($0,$1,$2,$3,$4,$5) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = +$4;
 $5 = $5|0;
 var $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0.0, $22 = 0, $23 = 0, $24 = 0.0, $25 = 0.0, $26 = 0.0, $27 = 0, $28 = 0.0, $29 = 0.0;
 var $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0.0, $39 = 0, $40 = 0, $41 = 0.0, $42 = 0.0, $43 = 0, $44 = 0, $45 = 0.0, $46 = 0, $47 = 0, $48 = 0.0, $49 = 0.0;
 var $50 = 0, $51 = 0.0, $52 = 0, $53 = 0.0, $54 = 0.0, $55 = 0, $56 = 0, $57 = 0, $58 = 0.0, $59 = 0, $6 = 0, $60 = 0.0, $61 = 0.0, $62 = 0, $63 = 0, $64 = 0.0, $65 = 0, $66 = 0, $67 = 0, $68 = 0.0;
 var $69 = 0, $7 = 0, $70 = 0.0, $71 = 0.0, $72 = 0, $73 = 0, $74 = 0, $75 = 0.0, $76 = 0, $77 = 0.0, $78 = 0.0, $79 = 0, $8 = 0, $80 = 0, $81 = 0.0, $82 = 0, $83 = 0.0, $84 = 0, $85 = 0, $86 = 0;
 var $87 = 0.0, $88 = 0, $89 = 0.0, $9 = 0, $90 = 0.0, $91 = 0, $92 = 0.0, $93 = 0, $94 = 0.0, $95 = 0.0, $96 = 0, $97 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $6 = HEAP32[$0>>2]|0;
 $7 = ($6|0)==(0);
 if ($7) {
  return;
 }
 $8 = ((($1)) + 8|0);
 $9 = HEAP32[$8>>2]|0;
 $10 = ($9|0)<(0);
 if ($10) {
  $11 = HEAP32[$1>>2]|0;
  $12 = (($11) - ($9))|0;
  HEAP32[$1>>2] = $12;
 }
 $13 = ((($1)) + 12|0);
 $14 = HEAP32[$13>>2]|0;
 $15 = ($14|0)<(0);
 if ($15) {
  $16 = ((($1)) + 4|0);
  $17 = HEAP32[$16>>2]|0;
  $18 = (($17) - ($14))|0;
  HEAP32[$16>>2] = $18;
 }
 $19 = HEAP32[$0>>2]|0;
 _rlEnableTexture($19);
 _rlPushMatrix();
 $20 = HEAP32[$2>>2]|0;
 $21 = (+($20|0));
 $22 = ((($2)) + 4|0);
 $23 = HEAP32[$22>>2]|0;
 $24 = (+($23|0));
 _rlTranslatef($21,$24,0.0);
 _rlRotatef($4,0.0,0.0,1.0);
 $25 = +HEAPF32[$3>>2];
 $26 = -$25;
 $27 = ((($3)) + 4|0);
 $28 = +HEAPF32[$27>>2];
 $29 = -$28;
 _rlTranslatef($26,$29,0.0);
 _rlBegin(7);
 $30 = HEAP8[$5>>0]|0;
 $31 = ((($5)) + 1|0);
 $32 = HEAP8[$31>>0]|0;
 $33 = ((($5)) + 2|0);
 $34 = HEAP8[$33>>0]|0;
 $35 = ((($5)) + 3|0);
 $36 = HEAP8[$35>>0]|0;
 _rlColor4ub($30,$32,$34,$36);
 $37 = HEAP32[$1>>2]|0;
 $38 = (+($37|0));
 $39 = ((($0)) + 4|0);
 $40 = HEAP32[$39>>2]|0;
 $41 = (+($40|0));
 $42 = $38 / $41;
 $43 = ((($1)) + 4|0);
 $44 = HEAP32[$43>>2]|0;
 $45 = (+($44|0));
 $46 = ((($0)) + 8|0);
 $47 = HEAP32[$46>>2]|0;
 $48 = (+($47|0));
 $49 = $45 / $48;
 _rlTexCoord2f($42,$49);
 _rlVertex2f(0.0,0.0);
 $50 = HEAP32[$1>>2]|0;
 $51 = (+($50|0));
 $52 = HEAP32[$39>>2]|0;
 $53 = (+($52|0));
 $54 = $51 / $53;
 $55 = HEAP32[$43>>2]|0;
 $56 = HEAP32[$13>>2]|0;
 $57 = (($56) + ($55))|0;
 $58 = (+($57|0));
 $59 = HEAP32[$46>>2]|0;
 $60 = (+($59|0));
 $61 = $58 / $60;
 _rlTexCoord2f($54,$61);
 $62 = ((($2)) + 12|0);
 $63 = HEAP32[$62>>2]|0;
 $64 = (+($63|0));
 _rlVertex2f(0.0,$64);
 $65 = HEAP32[$1>>2]|0;
 $66 = HEAP32[$8>>2]|0;
 $67 = (($66) + ($65))|0;
 $68 = (+($67|0));
 $69 = HEAP32[$39>>2]|0;
 $70 = (+($69|0));
 $71 = $68 / $70;
 $72 = HEAP32[$43>>2]|0;
 $73 = HEAP32[$13>>2]|0;
 $74 = (($73) + ($72))|0;
 $75 = (+($74|0));
 $76 = HEAP32[$46>>2]|0;
 $77 = (+($76|0));
 $78 = $75 / $77;
 _rlTexCoord2f($71,$78);
 $79 = ((($2)) + 8|0);
 $80 = HEAP32[$79>>2]|0;
 $81 = (+($80|0));
 $82 = HEAP32[$62>>2]|0;
 $83 = (+($82|0));
 _rlVertex2f($81,$83);
 $84 = HEAP32[$1>>2]|0;
 $85 = HEAP32[$8>>2]|0;
 $86 = (($85) + ($84))|0;
 $87 = (+($86|0));
 $88 = HEAP32[$39>>2]|0;
 $89 = (+($88|0));
 $90 = $87 / $89;
 $91 = HEAP32[$43>>2]|0;
 $92 = (+($91|0));
 $93 = HEAP32[$46>>2]|0;
 $94 = (+($93|0));
 $95 = $92 / $94;
 _rlTexCoord2f($90,$95);
 $96 = HEAP32[$79>>2]|0;
 $97 = (+($96|0));
 _rlVertex2f($97,0.0);
 _rlEnd();
 _rlPopMatrix();
 _rlDisableTexture();
 return;
}
function _DrawText($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $$ = 0, $$byval_copy = 0, $$byval_copy1 = 0, $$byval_copy2 = 0, $10 = 0.0, $11 = 0, $12 = 0.0, $13 = 0, $14 = 0, $15 = 0.0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 128|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(128|0);
 $$byval_copy2 = sp + 112|0;
 $$byval_copy1 = sp + 104|0;
 $$byval_copy = sp + 72|0;
 $5 = sp + 32|0;
 $6 = sp + 64|0;
 $7 = sp;
 _GetDefaultFont($5);
 $8 = HEAP32[$5>>2]|0;
 $9 = ($8|0)==(0);
 if ($9) {
  STACKTOP = sp;return;
 }
 $10 = (+($1|0));
 HEAPF32[$6>>2] = $10;
 $11 = ((($6)) + 4|0);
 $12 = (+($2|0));
 HEAPF32[$11>>2] = $12;
 $13 = ($3|0)>(10);
 $$ = $13 ? $3 : 10;
 $14 = (($$>>>0) / 10)&-1;
 _GetDefaultFont($7);
 $15 = (+($$|0));
 ;HEAP32[$$byval_copy>>2]=HEAP32[$7>>2]|0;HEAP32[$$byval_copy+4>>2]=HEAP32[$7+4>>2]|0;HEAP32[$$byval_copy+8>>2]=HEAP32[$7+8>>2]|0;HEAP32[$$byval_copy+12>>2]=HEAP32[$7+12>>2]|0;HEAP32[$$byval_copy+16>>2]=HEAP32[$7+16>>2]|0;HEAP32[$$byval_copy+20>>2]=HEAP32[$7+20>>2]|0;HEAP32[$$byval_copy+24>>2]=HEAP32[$7+24>>2]|0;HEAP32[$$byval_copy+28>>2]=HEAP32[$7+28>>2]|0;
 ;HEAP32[$$byval_copy1>>2]=HEAP32[$6>>2]|0;HEAP32[$$byval_copy1+4>>2]=HEAP32[$6+4>>2]|0;
 ;HEAP8[$$byval_copy2>>0]=HEAP8[$4>>0]|0;HEAP8[$$byval_copy2+1>>0]=HEAP8[$4+1>>0]|0;HEAP8[$$byval_copy2+2>>0]=HEAP8[$4+2>>0]|0;HEAP8[$$byval_copy2+3>>0]=HEAP8[$4+3>>0]|0;
 _DrawTextEx($$byval_copy,$0,$$byval_copy1,$15,$14,$$byval_copy2);
 STACKTOP = sp;return;
}
function _DrawTextEx($0,$1,$2,$3,$4,$5) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = +$3;
 $4 = $4|0;
 $5 = $5|0;
 var $$04954 = 0, $$05153 = 0, $$055 = 0, $$1 = 0, $$150 = 0, $$152 = 0, $$2 = 0, $$byval_copy1 = 0, $$byval_copy2 = 0, $$byval_copy3 = 0, $$byval_copy4 = 0, $$byval_copy5 = 0, $$sink = 0, $10 = 0, $11 = 0.0, $12 = 0.0, $13 = 0, $14 = 0, $15 = 0.0, $16 = 0;
 var $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0.0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0.0, $28 = 0.0, $29 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0;
 var $37 = 0, $38 = 0, $39 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0.0, $45 = 0.0, $46 = 0, $47 = 0, $48 = 0.0, $49 = 0.0, $50 = 0.0, $51 = 0, $52 = 0.0, $53 = 0.0, $54 = 0.0, $55 = 0, $56 = 0;
 var $57 = 0.0, $58 = 0.0, $59 = 0.0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0.0, $64 = 0.0, $65 = 0, $66 = 0, $67 = 0, $68 = 0.0, $69 = 0.0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0;
 var $75 = 0, $76 = 0, $77 = 0.0, $78 = 0.0, $79 = 0.0, $8 = 0, $80 = 0, $81 = 0, $82 = 0.0, $83 = 0.0, $84 = 0.0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 128|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(128|0);
 $$byval_copy5 = sp + 88|0;
 $$byval_copy4 = sp + 80|0;
 $$byval_copy3 = sp + 64|0;
 $$byval_copy2 = sp + 48|0;
 $$byval_copy1 = sp + 24|0;
 $6 = sp + 8|0;
 $7 = sp;
 $8 = (_strlen($1)|0);
 $9 = ((($0)) + 20|0);
 $10 = HEAP32[$9>>2]|0;
 $11 = (+($10|0));
 $12 = $3 / $11;
 $13 = ($8|0)>(0);
 if (!($13)) {
  STACKTOP = sp;return;
 }
 $14 = ((($0)) + 28|0);
 $15 = +HEAPF32[$2>>2];
 $16 = ((($6)) + 4|0);
 $17 = ((($2)) + 4|0);
 $18 = ((($6)) + 8|0);
 $19 = ((($6)) + 12|0);
 $20 = ((($7)) + 4|0);
 $21 = (+($4|0));
 $$04954 = 0;$$05153 = 0;$$055 = 0;
 while(1) {
  $22 = (($1) + ($$055)|0);
  $23 = HEAP8[$22>>0]|0;
  switch ($23<<24>>24) {
  case 10:  {
   $24 = HEAP32[$9>>2]|0;
   $25 = (($24|0) / 2)&-1;
   $26 = (($25) + ($24))|0;
   $27 = (+($26|0));
   $28 = $12 * $27;
   $29 = (~~(($28)));
   $30 = (($29) + ($$05153))|0;
   $$150 = 0;$$152 = $30;$$2 = $$055;
   break;
  }
  case -62:  {
   $31 = (($$055) + 1)|0;
   $32 = (($1) + ($31)|0);
   $33 = HEAP8[$32>>0]|0;
   $34 = $33&255;
   $$1 = $31;$$sink = $34;
   label = 9;
   break;
  }
  case -61:  {
   $35 = (($$055) + 1)|0;
   $36 = (($1) + ($35)|0);
   $37 = HEAP8[$36>>0]|0;
   $38 = $37&255;
   $39 = (($38) + 64)|0;
   $$1 = $35;$$sink = $39;
   label = 9;
   break;
  }
  default: {
   $40 = $23 << 24 >> 24;
   $$1 = $$055;$$sink = $40;
   label = 9;
  }
  }
  do {
   if ((label|0) == 9) {
    label = 0;
    ;HEAP32[$$byval_copy5>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy5+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy5+8>>2]=HEAP32[$0+8>>2]|0;HEAP32[$$byval_copy5+12>>2]=HEAP32[$0+12>>2]|0;HEAP32[$$byval_copy5+16>>2]=HEAP32[$0+16>>2]|0;HEAP32[$$byval_copy5+20>>2]=HEAP32[$0+20>>2]|0;HEAP32[$$byval_copy5+24>>2]=HEAP32[$0+24>>2]|0;HEAP32[$$byval_copy5+28>>2]=HEAP32[$0+28>>2]|0;
    $41 = (_GetCharIndex($$byval_copy5,$$sink)|0);
    $42 = HEAP32[$14>>2]|0;
    $43 = (((($42) + ($41<<5)|0)) + 4|0);
    $44 = (+($$04954|0));
    $45 = $44 + $15;
    $46 = (((($42) + ($41<<5)|0)) + 20|0);
    $47 = HEAP32[$46>>2]|0;
    $48 = (+($47|0));
    $49 = $12 * $48;
    $50 = $45 + $49;
    $51 = (~~(($50)));
    HEAP32[$6>>2] = $51;
    $52 = +HEAPF32[$17>>2];
    $53 = (+($$05153|0));
    $54 = $53 + $52;
    $55 = (((($42) + ($41<<5)|0)) + 24|0);
    $56 = HEAP32[$55>>2]|0;
    $57 = (+($56|0));
    $58 = $12 * $57;
    $59 = $54 + $58;
    $60 = (~~(($59)));
    HEAP32[$16>>2] = $60;
    $61 = (((($42) + ($41<<5)|0)) + 12|0);
    $62 = HEAP32[$61>>2]|0;
    $63 = (+($62|0));
    $64 = $12 * $63;
    $65 = (~~(($64)));
    HEAP32[$18>>2] = $65;
    $66 = (((($42) + ($41<<5)|0)) + 16|0);
    $67 = HEAP32[$66>>2]|0;
    $68 = (+($67|0));
    $69 = $12 * $68;
    $70 = (~~(($69)));
    HEAP32[$19>>2] = $70;
    HEAPF32[$7>>2] = 0.0;
    HEAPF32[$20>>2] = 0.0;
    ;HEAP32[$$byval_copy1>>2]=HEAP32[$0>>2]|0;HEAP32[$$byval_copy1+4>>2]=HEAP32[$0+4>>2]|0;HEAP32[$$byval_copy1+8>>2]=HEAP32[$0+8>>2]|0;HEAP32[$$byval_copy1+12>>2]=HEAP32[$0+12>>2]|0;HEAP32[$$byval_copy1+16>>2]=HEAP32[$0+16>>2]|0;
    ;HEAP32[$$byval_copy2>>2]=HEAP32[$43>>2]|0;HEAP32[$$byval_copy2+4>>2]=HEAP32[$43+4>>2]|0;HEAP32[$$byval_copy2+8>>2]=HEAP32[$43+8>>2]|0;HEAP32[$$byval_copy2+12>>2]=HEAP32[$43+12>>2]|0;
    ;HEAP32[$$byval_copy3>>2]=HEAP32[$6>>2]|0;HEAP32[$$byval_copy3+4>>2]=HEAP32[$6+4>>2]|0;HEAP32[$$byval_copy3+8>>2]=HEAP32[$6+8>>2]|0;HEAP32[$$byval_copy3+12>>2]=HEAP32[$6+12>>2]|0;
    ;HEAP32[$$byval_copy4>>2]=HEAP32[$7>>2]|0;HEAP32[$$byval_copy4+4>>2]=HEAP32[$7+4>>2]|0;
    ;HEAP8[$$byval_copy5>>0]=HEAP8[$5>>0]|0;HEAP8[$$byval_copy5+1>>0]=HEAP8[$5+1>>0]|0;HEAP8[$$byval_copy5+2>>0]=HEAP8[$5+2>>0]|0;HEAP8[$$byval_copy5+3>>0]=HEAP8[$5+3>>0]|0;
    _DrawTexturePro($$byval_copy1,$$byval_copy2,$$byval_copy3,$$byval_copy4,0.0,$$byval_copy5);
    $71 = HEAP32[$14>>2]|0;
    $72 = (((($71) + ($41<<5)|0)) + 28|0);
    $73 = HEAP32[$72>>2]|0;
    $74 = ($73|0)==(0);
    if ($74) {
     $75 = (((($71) + ($41<<5)|0)) + 12|0);
     $76 = HEAP32[$75>>2]|0;
     $77 = (+($76|0));
     $78 = $12 * $77;
     $79 = $21 + $78;
     $80 = (~~(($79)));
     $81 = (($80) + ($$04954))|0;
     $$150 = $81;$$152 = $$05153;$$2 = $$1;
     break;
    } else {
     $82 = (+($73|0));
     $83 = $12 * $82;
     $84 = $21 + $83;
     $85 = (~~(($84)));
     $86 = (($85) + ($$04954))|0;
     $$150 = $86;$$152 = $$05153;$$2 = $$1;
     break;
    }
   }
  } while(0);
  $87 = (($$2) + 1)|0;
  $88 = ($87|0)<($8|0);
  if ($88) {
   $$04954 = $$150;$$05153 = $$152;$$055 = $87;
  } else {
   break;
  }
 }
 STACKTOP = sp;return;
}
function _FormatText($0,$varargs) {
 $0 = $0|0;
 $varargs = $varargs|0;
 var $1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $1 = sp;
 HEAP32[$1>>2] = $varargs;
 (_vsprintf(18219,$0,$1)|0);
 STACKTOP = sp;return (18219|0);
}
function _emscripten_GetProcAddress($0) {
 $0 = $0|0;
 var $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0;
 var $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0;
 var $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0, $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0;
 var $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0, $16 = 0, $160 = 0, $161 = 0, $162 = 0, $163 = 0, $164 = 0, $165 = 0, $166 = 0, $167 = 0, $168 = 0, $169 = 0, $17 = 0, $170 = 0;
 var $171 = 0, $172 = 0, $173 = 0, $174 = 0, $175 = 0, $176 = 0, $177 = 0, $178 = 0, $179 = 0, $18 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0, $184 = 0, $185 = 0, $186 = 0, $187 = 0, $188 = 0, $189 = 0;
 var $19 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0, $197 = 0, $198 = 0, $199 = 0, $2 = 0, $20 = 0, $200 = 0, $201 = 0, $202 = 0, $203 = 0, $204 = 0, $205 = 0, $206 = 0;
 var $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0, $212 = 0, $213 = 0, $214 = 0, $215 = 0, $216 = 0, $217 = 0, $218 = 0, $219 = 0, $22 = 0, $220 = 0, $221 = 0, $222 = 0, $223 = 0, $224 = 0;
 var $225 = 0, $226 = 0, $227 = 0, $228 = 0, $229 = 0, $23 = 0, $230 = 0, $231 = 0, $232 = 0, $233 = 0, $234 = 0, $235 = 0, $236 = 0, $237 = 0, $238 = 0, $239 = 0, $24 = 0, $240 = 0, $241 = 0, $242 = 0;
 var $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0, $249 = 0, $25 = 0, $250 = 0, $251 = 0, $252 = 0, $253 = 0, $254 = 0, $255 = 0, $256 = 0, $257 = 0, $258 = 0, $259 = 0, $26 = 0, $260 = 0;
 var $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0, $267 = 0, $268 = 0, $269 = 0, $27 = 0, $270 = 0, $271 = 0, $272 = 0, $273 = 0, $274 = 0, $275 = 0, $276 = 0, $277 = 0, $278 = 0, $279 = 0;
 var $28 = 0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0, $285 = 0, $286 = 0, $287 = 0, $288 = 0, $289 = 0, $29 = 0, $290 = 0, $291 = 0, $292 = 0, $293 = 0, $294 = 0, $295 = 0, $296 = 0, $297 = 0;
 var $298 = 0, $299 = 0, $3 = 0, $30 = 0, $300 = 0, $301 = 0, $302 = 0, $303 = 0, $304 = 0, $305 = 0, $306 = 0, $307 = 0, $308 = 0, $309 = 0, $31 = 0, $310 = 0, $311 = 0, $312 = 0, $313 = 0, $314 = 0;
 var $315 = 0, $316 = 0, $317 = 0, $318 = 0, $319 = 0, $32 = 0, $320 = 0, $321 = 0, $322 = 0, $323 = 0, $324 = 0, $325 = 0, $326 = 0, $327 = 0, $328 = 0, $329 = 0, $33 = 0, $330 = 0, $331 = 0, $332 = 0;
 var $333 = 0, $334 = 0, $335 = 0, $336 = 0, $337 = 0, $338 = 0, $339 = 0, $34 = 0, $340 = 0, $341 = 0, $342 = 0, $343 = 0, $344 = 0, $345 = 0, $346 = 0, $347 = 0, $348 = 0, $349 = 0, $35 = 0, $350 = 0;
 var $351 = 0, $352 = 0, $353 = 0, $354 = 0, $355 = 0, $356 = 0, $357 = 0, $358 = 0, $359 = 0, $36 = 0, $360 = 0, $361 = 0, $362 = 0, $363 = 0, $364 = 0, $365 = 0, $366 = 0, $367 = 0, $368 = 0, $369 = 0;
 var $37 = 0, $370 = 0, $371 = 0, $372 = 0, $373 = 0, $374 = 0, $375 = 0, $376 = 0, $377 = 0, $378 = 0, $379 = 0, $38 = 0, $380 = 0, $381 = 0, $382 = 0, $383 = 0, $384 = 0, $385 = 0, $386 = 0, $387 = 0;
 var $388 = 0, $389 = 0, $39 = 0, $390 = 0, $391 = 0, $392 = 0, $393 = 0, $394 = 0, $395 = 0, $396 = 0, $397 = 0, $398 = 0, $399 = 0, $4 = 0, $40 = 0, $400 = 0, $401 = 0, $402 = 0, $403 = 0, $404 = 0;
 var $405 = 0, $406 = 0, $407 = 0, $408 = 0, $409 = 0, $41 = 0, $410 = 0, $411 = 0, $412 = 0, $413 = 0, $414 = 0, $415 = 0, $416 = 0, $417 = 0, $418 = 0, $419 = 0, $42 = 0, $420 = 0, $421 = 0, $422 = 0;
 var $423 = 0, $424 = 0, $425 = 0, $426 = 0, $427 = 0, $428 = 0, $429 = 0, $43 = 0, $430 = 0, $431 = 0, $432 = 0, $433 = 0, $434 = 0, $435 = 0, $436 = 0, $437 = 0, $438 = 0, $439 = 0, $44 = 0, $440 = 0;
 var $441 = 0, $442 = 0, $443 = 0, $444 = 0, $445 = 0, $446 = 0, $447 = 0, $448 = 0, $449 = 0, $45 = 0, $450 = 0, $451 = 0, $452 = 0, $453 = 0, $454 = 0, $455 = 0, $456 = 0, $457 = 0, $458 = 0, $459 = 0;
 var $46 = 0, $460 = 0, $461 = 0, $462 = 0, $463 = 0, $464 = 0, $465 = 0, $466 = 0, $467 = 0, $468 = 0, $469 = 0, $47 = 0, $470 = 0, $471 = 0, $472 = 0, $473 = 0, $474 = 0, $475 = 0, $476 = 0, $477 = 0;
 var $478 = 0, $479 = 0, $48 = 0, $480 = 0, $481 = 0, $482 = 0, $483 = 0, $484 = 0, $485 = 0, $486 = 0, $487 = 0, $488 = 0, $489 = 0, $49 = 0, $490 = 0, $491 = 0, $492 = 0, $493 = 0, $494 = 0, $495 = 0;
 var $496 = 0, $497 = 0, $498 = 0, $499 = 0, $5 = 0, $50 = 0, $500 = 0, $501 = 0, $502 = 0, $503 = 0, $504 = 0, $505 = 0, $506 = 0, $507 = 0, $508 = 0, $509 = 0, $51 = 0, $510 = 0, $511 = 0, $512 = 0;
 var $513 = 0, $514 = 0, $515 = 0, $516 = 0, $517 = 0, $518 = 0, $519 = 0, $52 = 0, $520 = 0, $521 = 0, $522 = 0, $523 = 0, $524 = 0, $525 = 0, $526 = 0, $527 = 0, $528 = 0, $529 = 0, $53 = 0, $530 = 0;
 var $531 = 0, $532 = 0, $533 = 0, $534 = 0, $535 = 0, $536 = 0, $537 = 0, $538 = 0, $539 = 0, $54 = 0, $540 = 0, $541 = 0, $542 = 0, $543 = 0, $544 = 0, $545 = 0, $546 = 0, $547 = 0, $548 = 0, $549 = 0;
 var $55 = 0, $550 = 0, $551 = 0, $552 = 0, $553 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0;
 var $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0;
 var $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $1 = sp + 12|0;
 $2 = sp + 8|0;
 $3 = sp + 4|0;
 $4 = sp;
 HEAP32[$2>>2] = $0;
 $5 = HEAP32[$2>>2]|0;
 $6 = (_strlen($5)|0);
 $7 = (($6) + 1)|0;
 $8 = (_malloc($7)|0);
 HEAP32[$3>>2] = $8;
 $9 = HEAP32[$3>>2]|0;
 $10 = HEAP32[$2>>2]|0;
 (_strcpy($9,$10)|0);
 $11 = HEAP32[$3>>2]|0;
 $12 = (_strstr($11,8429)|0);
 HEAP32[$4>>2] = $12;
 $13 = HEAP32[$4>>2]|0;
 $14 = ($13|0)!=(0|0);
 if ($14) {
  $15 = HEAP32[$4>>2]|0;
  HEAP8[$15>>0] = 0;
 }
 $16 = HEAP32[$3>>2]|0;
 $17 = (_strstr($16,8433)|0);
 HEAP32[$4>>2] = $17;
 $18 = HEAP32[$4>>2]|0;
 $19 = ($18|0)!=(0|0);
 if ($19) {
  $20 = HEAP32[$4>>2]|0;
  HEAP8[$20>>0] = 0;
 }
 $21 = HEAP32[$3>>2]|0;
 $22 = (_strstr($21,8437)|0);
 HEAP32[$4>>2] = $22;
 $23 = HEAP32[$4>>2]|0;
 $24 = ($23|0)!=(0|0);
 if ($24) {
  $25 = HEAP32[$4>>2]|0;
  HEAP8[$25>>0] = 0;
 }
 $26 = HEAP32[$3>>2]|0;
 $27 = (_strstr($26,8441)|0);
 HEAP32[$4>>2] = $27;
 $28 = HEAP32[$4>>2]|0;
 $29 = ($28|0)!=(0|0);
 if ($29) {
  $30 = HEAP32[$4>>2]|0;
  HEAP8[$30>>0] = 0;
 }
 $31 = HEAP32[$3>>2]|0;
 $32 = (_strcmp($31,8447)|0);
 $33 = ($32|0)!=(0);
 do {
  if ($33) {
   $34 = HEAP32[$3>>2]|0;
   $35 = (_strcmp($34,8485)|0);
   $36 = ($35|0)!=(0);
   if (!($36)) {
    HEAP32[$3>>2] = 8504;
    break;
   }
   $37 = HEAP32[$3>>2]|0;
   $38 = (_strcmp($37,8517)|0);
   $39 = ($38|0)!=(0);
   if (!($39)) {
    HEAP32[$3>>2] = 8538;
    break;
   }
   $40 = HEAP32[$3>>2]|0;
   $41 = (_strcmp($40,8553)|0);
   $42 = ($41|0)!=(0);
   if (!($42)) {
    HEAP32[$3>>2] = 8568;
    break;
   }
   $43 = HEAP32[$3>>2]|0;
   $44 = (_strcmp($43,8583)|0);
   $45 = ($44|0)!=(0);
   if (!($45)) {
    HEAP32[$3>>2] = 8598;
   }
  } else {
   HEAP32[$3>>2] = 8469;
  }
 } while(0);
 $46 = HEAP32[$3>>2]|0;
 $47 = (_strcmp($46,8613)|0);
 $48 = ($47|0)!=(0);
 do {
  if ($48) {
   $49 = HEAP32[$3>>2]|0;
   $50 = (_strcmp($49,8627)|0);
   $51 = ($50|0)!=(0);
   if (!($51)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $52 = HEAP32[$3>>2]|0;
   $53 = (_strcmp($52,8639)|0);
   $54 = ($53|0)!=(0);
   if (!($54)) {
    HEAP32[$1>>2] = 6;
    break;
   }
   $55 = HEAP32[$3>>2]|0;
   $56 = (_strcmp($55,8653)|0);
   $57 = ($56|0)!=(0);
   if (!($57)) {
    HEAP32[$1>>2] = 7;
    break;
   }
   $58 = HEAP32[$3>>2]|0;
   $59 = (_strcmp($58,8665)|0);
   $60 = ($59|0)!=(0);
   if (!($60)) {
    HEAP32[$1>>2] = 8;
    break;
   }
   $61 = HEAP32[$3>>2]|0;
   $62 = (_strcmp($61,8679)|0);
   $63 = ($62|0)!=(0);
   if (!($63)) {
    HEAP32[$1>>2] = 9;
    break;
   }
   $64 = HEAP32[$3>>2]|0;
   $65 = (_strcmp($64,8693)|0);
   $66 = ($65|0)!=(0);
   if (!($66)) {
    HEAP32[$1>>2] = 10;
    break;
   }
   $67 = HEAP32[$3>>2]|0;
   $68 = (_strcmp($67,8710)|0);
   $69 = ($68|0)!=(0);
   if (!($69)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $70 = HEAP32[$3>>2]|0;
   $71 = (_strcmp($70,8733)|0);
   $72 = ($71|0)!=(0);
   if (!($72)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $73 = HEAP32[$3>>2]|0;
   $74 = (_strcmp($73,8759)|0);
   $75 = ($74|0)!=(0);
   if (!($75)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $76 = HEAP32[$3>>2]|0;
   $77 = (_strcmp($76,8772)|0);
   $78 = ($77|0)!=(0);
   if (!($78)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $79 = HEAP32[$3>>2]|0;
   $80 = (_strcmp($79,8788)|0);
   $81 = ($80|0)!=(0);
   if (!($81)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $82 = HEAP32[$3>>2]|0;
   $83 = (_strcmp($82,8801)|0);
   $84 = ($83|0)!=(0);
   if (!($84)) {
    HEAP32[$1>>2] = 11;
    break;
   }
   $85 = HEAP32[$3>>2]|0;
   $86 = (_strcmp($85,8815)|0);
   $87 = ($86|0)!=(0);
   if (!($87)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $88 = HEAP32[$3>>2]|0;
   $89 = (_strcmp($88,8835)|0);
   $90 = ($89|0)!=(0);
   if (!($90)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $91 = HEAP32[$3>>2]|0;
   $92 = (_strcmp($91,8855)|0);
   $93 = ($92|0)!=(0);
   if (!($93)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $94 = HEAP32[$3>>2]|0;
   $95 = (_strcmp($94,8872)|0);
   $96 = ($95|0)!=(0);
   if (!($96)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $97 = HEAP32[$3>>2]|0;
   $98 = (_strcmp($97,8889)|0);
   $99 = ($98|0)!=(0);
   if (!($99)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $100 = HEAP32[$3>>2]|0;
   $101 = (_strcmp($100,8901)|0);
   $102 = ($101|0)!=(0);
   if (!($102)) {
    HEAP32[$1>>2] = 12;
    break;
   }
   $103 = HEAP32[$3>>2]|0;
   $104 = (_strcmp($103,8914)|0);
   $105 = ($104|0)!=(0);
   if (!($105)) {
    HEAP32[$1>>2] = 13;
    break;
   }
   $106 = HEAP32[$3>>2]|0;
   $107 = (_strcmp($106,8930)|0);
   $108 = ($107|0)!=(0);
   if (!($108)) {
    HEAP32[$1>>2] = 6;
    break;
   }
   $109 = HEAP32[$3>>2]|0;
   $110 = (_strcmp($109,8953)|0);
   $111 = ($110|0)!=(0);
   if (!($111)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $112 = HEAP32[$3>>2]|0;
   $113 = (_strcmp($112,8966)|0);
   $114 = ($113|0)!=(0);
   if (!($114)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $115 = HEAP32[$3>>2]|0;
   $116 = (_strcmp($115,8982)|0);
   $117 = ($116|0)!=(0);
   if (!($117)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $118 = HEAP32[$3>>2]|0;
   $119 = (_strcmp($118,8993)|0);
   $120 = ($119|0)!=(0);
   if (!($120)) {
    HEAP32[$1>>2] = 14;
    break;
   }
   $121 = HEAP32[$3>>2]|0;
   $122 = (_strcmp($121,9012)|0);
   $123 = ($122|0)!=(0);
   if (!($123)) {
    HEAP32[$1>>2] = 15;
    break;
   }
   $124 = HEAP32[$3>>2]|0;
   $125 = (_strcmp($124,9034)|0);
   $126 = ($125|0)!=(0);
   if (!($126)) {
    HEAP32[$1>>2] = 16;
    break;
   }
   $127 = HEAP32[$3>>2]|0;
   $128 = (_strcmp($127,9053)|0);
   $129 = ($128|0)!=(0);
   if (!($129)) {
    HEAP32[$1>>2] = 7;
    break;
   }
   $130 = HEAP32[$3>>2]|0;
   $131 = (_strcmp($130,9082)|0);
   $132 = ($131|0)!=(0);
   if (!($132)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $133 = HEAP32[$3>>2]|0;
   $134 = (_strcmp($133,9099)|0);
   $135 = ($134|0)!=(0);
   if (!($135)) {
    HEAP32[$1>>2] = 8;
    break;
   }
   $136 = HEAP32[$3>>2]|0;
   $137 = (_strcmp($136,9114)|0);
   $138 = ($137|0)!=(0);
   if (!($138)) {
    HEAP32[$1>>2] = 9;
    break;
   }
   $139 = HEAP32[$3>>2]|0;
   $140 = (_strcmp($139,9129)|0);
   $141 = ($140|0)!=(0);
   if (!($141)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $142 = HEAP32[$3>>2]|0;
   $143 = (_strcmp($142,9150)|0);
   $144 = ($143|0)!=(0);
   if (!($144)) {
    HEAP32[$1>>2] = 10;
    break;
   }
   $145 = HEAP32[$3>>2]|0;
   $146 = (_strcmp($145,9170)|0);
   $147 = ($146|0)!=(0);
   if (!($147)) {
    HEAP32[$1>>2] = 11;
    break;
   }
   $148 = HEAP32[$3>>2]|0;
   $149 = (_strcmp($148,9190)|0);
   $150 = ($149|0)!=(0);
   if (!($150)) {
    HEAP32[$1>>2] = 12;
    break;
   }
   $151 = HEAP32[$3>>2]|0;
   $152 = (_strcmp($151,9216)|0);
   $153 = ($152|0)!=(0);
   if (!($153)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $154 = HEAP32[$3>>2]|0;
   $155 = (_strcmp($154,9235)|0);
   $156 = ($155|0)!=(0);
   if (!($156)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $157 = HEAP32[$3>>2]|0;
   $158 = (_strcmp($157,9247)|0);
   $159 = ($158|0)!=(0);
   if (!($159)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $160 = HEAP32[$3>>2]|0;
   $161 = (_strcmp($160,9259)|0);
   $162 = ($161|0)!=(0);
   if (!($162)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $163 = HEAP32[$3>>2]|0;
   $164 = (_strcmp($163,9271)|0);
   $165 = ($164|0)!=(0);
   if (!($165)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $166 = HEAP32[$3>>2]|0;
   $167 = (_strcmp($166,9283)|0);
   $168 = ($167|0)!=(0);
   if (!($168)) {
    HEAP32[$1>>2] = 17;
    break;
   }
   $169 = HEAP32[$3>>2]|0;
   $170 = (_strcmp($169,9295)|0);
   $171 = ($170|0)!=(0);
   if (!($171)) {
    HEAP32[$1>>2] = 13;
    break;
   }
   $172 = HEAP32[$3>>2]|0;
   $173 = (_strcmp($172,9307)|0);
   $174 = ($173|0)!=(0);
   if (!($174)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $175 = HEAP32[$3>>2]|0;
   $176 = (_strcmp($175,9319)|0);
   $177 = ($176|0)!=(0);
   if (!($177)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $178 = HEAP32[$3>>2]|0;
   $179 = (_strcmp($178,9331)|0);
   $180 = ($179|0)!=(0);
   if (!($180)) {
    HEAP32[$1>>2] = 14;
    break;
   }
   $181 = HEAP32[$3>>2]|0;
   $182 = (_strcmp($181,9344)|0);
   $183 = ($182|0)!=(0);
   if (!($183)) {
    HEAP32[$1>>2] = 15;
    break;
   }
   $184 = HEAP32[$3>>2]|0;
   $185 = (_strcmp($184,9357)|0);
   $186 = ($185|0)!=(0);
   if (!($186)) {
    HEAP32[$1>>2] = 16;
    break;
   }
   $187 = HEAP32[$3>>2]|0;
   $188 = (_strcmp($187,9370)|0);
   $189 = ($188|0)!=(0);
   if (!($189)) {
    HEAP32[$1>>2] = 17;
    break;
   }
   $190 = HEAP32[$3>>2]|0;
   $191 = (_strcmp($190,9383)|0);
   $192 = ($191|0)!=(0);
   if (!($192)) {
    HEAP32[$1>>2] = 18;
    break;
   }
   $193 = HEAP32[$3>>2]|0;
   $194 = (_strcmp($193,9396)|0);
   $195 = ($194|0)!=(0);
   if (!($195)) {
    HEAP32[$1>>2] = 19;
    break;
   }
   $196 = HEAP32[$3>>2]|0;
   $197 = (_strcmp($196,9409)|0);
   $198 = ($197|0)!=(0);
   if (!($198)) {
    HEAP32[$1>>2] = 20;
    break;
   }
   $199 = HEAP32[$3>>2]|0;
   $200 = (_strcmp($199,9422)|0);
   $201 = ($200|0)!=(0);
   if (!($201)) {
    HEAP32[$1>>2] = 21;
    break;
   }
   $202 = HEAP32[$3>>2]|0;
   $203 = (_strcmp($202,9435)|0);
   $204 = ($203|0)!=(0);
   if (!($204)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $205 = HEAP32[$3>>2]|0;
   $206 = (_strcmp($205,9454)|0);
   $207 = ($206|0)!=(0);
   if (!($207)) {
    HEAP32[$1>>2] = 6;
    break;
   }
   $208 = HEAP32[$3>>2]|0;
   $209 = (_strcmp($208,9473)|0);
   $210 = ($209|0)!=(0);
   if (!($210)) {
    HEAP32[$1>>2] = 7;
    break;
   }
   $211 = HEAP32[$3>>2]|0;
   $212 = (_strcmp($211,9492)|0);
   $213 = ($212|0)!=(0);
   if (!($213)) {
    HEAP32[$1>>2] = 18;
    break;
   }
   $214 = HEAP32[$3>>2]|0;
   $215 = (_strcmp($214,9505)|0);
   $216 = ($215|0)!=(0);
   if (!($216)) {
    HEAP32[$1>>2] = 19;
    break;
   }
   $217 = HEAP32[$3>>2]|0;
   $218 = (_strcmp($217,9523)|0);
   $219 = ($218|0)!=(0);
   if (!($219)) {
    HEAP32[$1>>2] = 20;
    break;
   }
   $220 = HEAP32[$3>>2]|0;
   $221 = (_strcmp($220,9541)|0);
   $222 = ($221|0)!=(0);
   if (!($222)) {
    HEAP32[$1>>2] = 21;
    break;
   }
   $223 = HEAP32[$3>>2]|0;
   $224 = (_strcmp($223,9559)|0);
   $225 = ($224|0)!=(0);
   if (!($225)) {
    HEAP32[$1>>2] = 22;
    break;
   }
   $226 = HEAP32[$3>>2]|0;
   $227 = (_strcmp($226,9577)|0);
   $228 = ($227|0)!=(0);
   if (!($228)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $229 = HEAP32[$3>>2]|0;
   $230 = (_strcmp($229,9597)|0);
   $231 = ($230|0)!=(0);
   if (!($231)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $232 = HEAP32[$3>>2]|0;
   $233 = (_strcmp($232,8538)|0);
   $234 = ($233|0)!=(0);
   if (!($234)) {
    HEAP32[$1>>2] = 6;
    break;
   }
   $235 = HEAP32[$3>>2]|0;
   $236 = (_strcmp($235,9615)|0);
   $237 = ($236|0)!=(0);
   if (!($237)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $238 = HEAP32[$3>>2]|0;
   $239 = (_strcmp($238,9630)|0);
   $240 = ($239|0)!=(0);
   if (!($240)) {
    HEAP32[$1>>2] = 8;
    break;
   }
   $241 = HEAP32[$3>>2]|0;
   $242 = (_strcmp($241,9651)|0);
   $243 = ($242|0)!=(0);
   if (!($243)) {
    HEAP32[$1>>2] = 9;
    break;
   }
   $244 = HEAP32[$3>>2]|0;
   $245 = (_strcmp($244,9666)|0);
   $246 = ($245|0)!=(0);
   if (!($246)) {
    HEAP32[$1>>2] = 10;
    break;
   }
   $247 = HEAP32[$3>>2]|0;
   $248 = (_strcmp($247,9684)|0);
   $249 = ($248|0)!=(0);
   if (!($249)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $250 = HEAP32[$3>>2]|0;
   $251 = (_strcmp($250,9700)|0);
   $252 = ($251|0)!=(0);
   if (!($252)) {
    HEAP32[$1>>2] = 11;
    break;
   }
   $253 = HEAP32[$3>>2]|0;
   $254 = (_strcmp($253,9719)|0);
   $255 = ($254|0)!=(0);
   if (!($255)) {
    HEAP32[$1>>2] = 22;
    break;
   }
   $256 = HEAP32[$3>>2]|0;
   $257 = (_strcmp($256,9733)|0);
   $258 = ($257|0)!=(0);
   if (!($258)) {
    HEAP32[$1>>2] = 23;
    break;
   }
   $259 = HEAP32[$3>>2]|0;
   $260 = (_strcmp($259,9748)|0);
   $261 = ($260|0)!=(0);
   if (!($261)) {
    HEAP32[$1>>2] = 7;
    break;
   }
   $262 = HEAP32[$3>>2]|0;
   $263 = (_strcmp($262,8469)|0);
   $264 = ($263|0)!=(0);
   if (!($264)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $265 = HEAP32[$3>>2]|0;
   $266 = (_strcmp($265,9759)|0);
   $267 = ($266|0)!=(0);
   if (!($267)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $268 = HEAP32[$3>>2]|0;
   $269 = (_strcmp($268,8568)|0);
   $270 = ($269|0)!=(0);
   if (!($270)) {
    HEAP32[$1>>2] = 23;
    break;
   }
   $271 = HEAP32[$3>>2]|0;
   $272 = (_strcmp($271,8598)|0);
   $273 = ($272|0)!=(0);
   if (!($273)) {
    HEAP32[$1>>2] = 24;
    break;
   }
   $274 = HEAP32[$3>>2]|0;
   $275 = (_strcmp($274,9775)|0);
   $276 = ($275|0)!=(0);
   if (!($276)) {
    HEAP32[$1>>2] = 12;
    break;
   }
   $277 = HEAP32[$3>>2]|0;
   $278 = (_strcmp($277,9802)|0);
   $279 = ($278|0)!=(0);
   if (!($279)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $280 = HEAP32[$3>>2]|0;
   $281 = (_strcmp($280,9816)|0);
   $282 = ($281|0)!=(0);
   if (!($282)) {
    HEAP32[$1>>2] = 13;
    break;
   }
   $283 = HEAP32[$3>>2]|0;
   $284 = (_strcmp($283,8504)|0);
   $285 = ($284|0)!=(0);
   if (!($285)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $286 = HEAP32[$3>>2]|0;
   $287 = (_strcmp($286,9836)|0);
   $288 = ($287|0)!=(0);
   if (!($288)) {
    HEAP32[$1>>2] = 6;
    break;
   }
   $289 = HEAP32[$3>>2]|0;
   $290 = (_strcmp($289,9854)|0);
   $291 = ($290|0)!=(0);
   if (!($291)) {
    HEAP32[$1>>2] = 8;
    break;
   }
   $292 = HEAP32[$3>>2]|0;
   $293 = (_strcmp($292,9866)|0);
   $294 = ($293|0)!=(0);
   if (!($294)) {
    HEAP32[$1>>2] = 24;
    break;
   }
   $295 = HEAP32[$3>>2]|0;
   $296 = (_strcmp($295,9887)|0);
   $297 = ($296|0)!=(0);
   if (!($297)) {
    HEAP32[$1>>2] = 25;
    break;
   }
   $298 = HEAP32[$3>>2]|0;
   $299 = (_strcmp($298,9905)|0);
   $300 = ($299|0)!=(0);
   if (!($300)) {
    HEAP32[$1>>2] = 26;
    break;
   }
   $301 = HEAP32[$3>>2]|0;
   $302 = (_strcmp($301,9923)|0);
   $303 = ($302|0)!=(0);
   if (!($303)) {
    HEAP32[$1>>2] = 27;
    break;
   }
   $304 = HEAP32[$3>>2]|0;
   $305 = (_strcmp($304,9944)|0);
   $306 = ($305|0)!=(0);
   if (!($306)) {
    HEAP32[$1>>2] = 14;
    break;
   }
   $307 = HEAP32[$3>>2]|0;
   $308 = (_strcmp($307,9970)|0);
   $309 = ($308|0)!=(0);
   if (!($309)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $310 = HEAP32[$3>>2]|0;
   $311 = (_strcmp($310,9993)|0);
   $312 = ($311|0)!=(0);
   if (!($312)) {
    HEAP32[$1>>2] = 15;
    break;
   }
   $313 = HEAP32[$3>>2]|0;
   $314 = (_strcmp($313,10031)|0);
   $315 = ($314|0)!=(0);
   if (!($315)) {
    HEAP32[$1>>2] = 9;
    break;
   }
   $316 = HEAP32[$3>>2]|0;
   $317 = (_strcmp($316,10047)|0);
   $318 = ($317|0)!=(0);
   if (!($318)) {
    HEAP32[$1>>2] = 7;
    break;
   }
   $319 = HEAP32[$3>>2]|0;
   $320 = (_strcmp($319,10062)|0);
   $321 = ($320|0)!=(0);
   if (!($321)) {
    HEAP32[$1>>2] = 25;
    break;
   }
   $322 = HEAP32[$3>>2]|0;
   $323 = (_strcmp($322,10085)|0);
   $324 = ($323|0)!=(0);
   if (!($324)) {
    HEAP32[$1>>2] = 16;
    break;
   }
   $325 = HEAP32[$3>>2]|0;
   $326 = (_strcmp($325,10098)|0);
   $327 = ($326|0)!=(0);
   if (!($327)) {
    HEAP32[$1>>2] = 28;
    break;
   }
   $328 = HEAP32[$3>>2]|0;
   $329 = (_strcmp($328,10112)|0);
   $330 = ($329|0)!=(0);
   if (!($330)) {
    HEAP32[$1>>2] = 29;
    break;
   }
   $331 = HEAP32[$3>>2]|0;
   $332 = (_strcmp($331,10126)|0);
   $333 = ($332|0)!=(0);
   if (!($333)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $334 = HEAP32[$3>>2]|0;
   $335 = (_strcmp($334,10146)|0);
   $336 = ($335|0)!=(0);
   if (!($336)) {
    HEAP32[$1>>2] = 8;
    break;
   }
   $337 = HEAP32[$3>>2]|0;
   $338 = (_strcmp($337,10166)|0);
   $339 = ($338|0)!=(0);
   if (!($339)) {
    HEAP32[$1>>2] = 17;
    break;
   }
   $340 = HEAP32[$3>>2]|0;
   $341 = (_strcmp($340,10182)|0);
   $342 = ($341|0)!=(0);
   if (!($342)) {
    HEAP32[$1>>2] = 18;
    break;
   }
   $343 = HEAP32[$3>>2]|0;
   $344 = (_strcmp($343,10200)|0);
   $345 = ($344|0)!=(0);
   if (!($345)) {
    HEAP32[$1>>2] = 26;
    break;
   }
   $346 = HEAP32[$3>>2]|0;
   $347 = (_strcmp($346,10216)|0);
   $348 = ($347|0)!=(0);
   if (!($348)) {
    HEAP32[$1>>2] = 19;
    break;
   }
   $349 = HEAP32[$3>>2]|0;
   $350 = (_strcmp($349,10231)|0);
   $351 = ($350|0)!=(0);
   if (!($351)) {
    HEAP32[$1>>2] = 9;
    break;
   }
   $352 = HEAP32[$3>>2]|0;
   $353 = (_strcmp($352,10253)|0);
   $354 = ($353|0)!=(0);
   if (!($354)) {
    HEAP32[$1>>2] = 30;
    break;
   }
   $355 = HEAP32[$3>>2]|0;
   $356 = (_strcmp($355,10271)|0);
   $357 = ($356|0)!=(0);
   if (!($357)) {
    HEAP32[$1>>2] = 31;
    break;
   }
   $358 = HEAP32[$3>>2]|0;
   $359 = (_strcmp($358,10292)|0);
   $360 = ($359|0)!=(0);
   if (!($360)) {
    HEAP32[$1>>2] = 10;
    break;
   }
   $361 = HEAP32[$3>>2]|0;
   $362 = (_strcmp($361,10310)|0);
   $363 = ($362|0)!=(0);
   if (!($363)) {
    HEAP32[$1>>2] = 11;
    break;
   }
   $364 = HEAP32[$3>>2]|0;
   $365 = (_strcmp($364,10323)|0);
   $366 = ($365|0)!=(0);
   if (!($366)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $367 = HEAP32[$3>>2]|0;
   $368 = (_strcmp($367,10338)|0);
   $369 = ($368|0)!=(0);
   if (!($369)) {
    HEAP32[$1>>2] = 12;
    break;
   }
   $370 = HEAP32[$3>>2]|0;
   $371 = (_strcmp($370,10352)|0);
   $372 = ($371|0)!=(0);
   if (!($372)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $373 = HEAP32[$3>>2]|0;
   $374 = (_strcmp($373,10362)|0);
   $375 = ($374|0)!=(0);
   if (!($375)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $376 = HEAP32[$3>>2]|0;
   $377 = (_strcmp($376,10372)|0);
   $378 = ($377|0)!=(0);
   if (!($378)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $379 = HEAP32[$3>>2]|0;
   $380 = (_strcmp($379,10394)|0);
   $381 = ($380|0)!=(0);
   if (!($381)) {
    HEAP32[$1>>2] = 13;
    break;
   }
   $382 = HEAP32[$3>>2]|0;
   $383 = (_strcmp($382,10420)|0);
   $384 = ($383|0)!=(0);
   if (!($384)) {
    HEAP32[$1>>2] = 14;
    break;
   }
   $385 = HEAP32[$3>>2]|0;
   $386 = (_strcmp($385,10447)|0);
   $387 = ($386|0)!=(0);
   if (!($387)) {
    HEAP32[$1>>2] = 27;
    break;
   }
   $388 = HEAP32[$3>>2]|0;
   $389 = (_strcmp($388,10460)|0);
   $390 = ($389|0)!=(0);
   if (!($390)) {
    HEAP32[$1>>2] = 20;
    break;
   }
   $391 = HEAP32[$3>>2]|0;
   $392 = (_strcmp($391,10475)|0);
   $393 = ($392|0)!=(0);
   if (!($393)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $394 = HEAP32[$3>>2]|0;
   $395 = (_strcmp($394,10490)|0);
   $396 = ($395|0)!=(0);
   if (!($396)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $397 = HEAP32[$3>>2]|0;
   $398 = (_strcmp($397,10514)|0);
   $399 = ($398|0)!=(0);
   if (!($399)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $400 = HEAP32[$3>>2]|0;
   $401 = (_strcmp($400,10525)|0);
   $402 = ($401|0)!=(0);
   if (!($402)) {
    HEAP32[$1>>2] = 32;
    break;
   }
   $403 = HEAP32[$3>>2]|0;
   $404 = (_strcmp($403,10547)|0);
   $405 = ($404|0)!=(0);
   if (!($405)) {
    HEAP32[$1>>2] = 21;
    break;
   }
   $406 = HEAP32[$3>>2]|0;
   $407 = (_strcmp($406,10569)|0);
   $408 = ($407|0)!=(0);
   if (!($408)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $409 = HEAP32[$3>>2]|0;
   $410 = (_strcmp($409,10593)|0);
   $411 = ($410|0)!=(0);
   if (!($411)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $412 = HEAP32[$3>>2]|0;
   $413 = (_strcmp($412,10602)|0);
   $414 = ($413|0)!=(0);
   if (!($414)) {
    HEAP32[$1>>2] = 5;
    break;
   }
   $415 = HEAP32[$3>>2]|0;
   $416 = (_strcmp($415,10610)|0);
   $417 = ($416|0)!=(0);
   if (!($417)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $418 = HEAP32[$3>>2]|0;
   $419 = (_strcmp($418,10623)|0);
   $420 = ($419|0)!=(0);
   if (!($420)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $421 = HEAP32[$3>>2]|0;
   $422 = (_strcmp($421,10637)|0);
   $423 = ($422|0)!=(0);
   if (!($423)) {
    HEAP32[$1>>2] = 15;
    break;
   }
   $424 = HEAP32[$3>>2]|0;
   $425 = (_strcmp($424,10649)|0);
   $426 = ($425|0)!=(0);
   if (!($426)) {
    HEAP32[$1>>2] = 16;
    break;
   }
   $427 = HEAP32[$3>>2]|0;
   $428 = (_strcmp($427,10658)|0);
   $429 = ($428|0)!=(0);
   if (!($429)) {
    HEAP32[$1>>2] = 17;
    break;
   }
   $430 = HEAP32[$3>>2]|0;
   $431 = (_strcmp($430,10668)|0);
   $432 = ($431|0)!=(0);
   if (!($432)) {
    HEAP32[$1>>2] = 18;
    break;
   }
   $433 = HEAP32[$3>>2]|0;
   $434 = (_strcmp($433,10680)|0);
   $435 = ($434|0)!=(0);
   if (!($435)) {
    HEAP32[$1>>2] = 19;
    break;
   }
   $436 = HEAP32[$3>>2]|0;
   $437 = (_strcmp($436,10691)|0);
   $438 = ($437|0)!=(0);
   if (!($438)) {
    HEAP32[$1>>2] = 20;
    break;
   }
   $439 = HEAP32[$3>>2]|0;
   $440 = (_strcmp($439,10699)|0);
   $441 = ($440|0)!=(0);
   if (!($441)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $442 = HEAP32[$3>>2]|0;
   $443 = (_strcmp($442,10711)|0);
   $444 = ($443|0)!=(0);
   if (!($444)) {
    HEAP32[$1>>2] = 21;
    break;
   }
   $445 = HEAP32[$3>>2]|0;
   $446 = (_strcmp($445,10726)|0);
   $447 = ($446|0)!=(0);
   if (!($447)) {
    HEAP32[$1>>2] = 22;
    break;
   }
   $448 = HEAP32[$3>>2]|0;
   $449 = (_strcmp($448,10738)|0);
   $450 = ($449|0)!=(0);
   if (!($450)) {
    HEAP32[$1>>2] = 23;
    break;
   }
   $451 = HEAP32[$3>>2]|0;
   $452 = (_strcmp($451,10752)|0);
   $453 = ($452|0)!=(0);
   if (!($453)) {
    HEAP32[$1>>2] = 10;
    break;
   }
   $454 = HEAP32[$3>>2]|0;
   $455 = (_strcmp($454,10777)|0);
   $456 = ($455|0)!=(0);
   if (!($456)) {
    HEAP32[$1>>2] = 24;
    break;
   }
   $457 = HEAP32[$3>>2]|0;
   $458 = (_strcmp($457,10794)|0);
   $459 = ($458|0)!=(0);
   if (!($459)) {
    HEAP32[$1>>2] = 25;
    break;
   }
   $460 = HEAP32[$3>>2]|0;
   $461 = (_strcmp($460,10810)|0);
   $462 = ($461|0)!=(0);
   if (!($462)) {
    HEAP32[$1>>2] = 26;
    break;
   }
   $463 = HEAP32[$3>>2]|0;
   $464 = (_strcmp($463,10826)|0);
   $465 = ($464|0)!=(0);
   if (!($465)) {
    HEAP32[$1>>2] = 11;
    break;
   }
   $466 = HEAP32[$3>>2]|0;
   $467 = (_strcmp($466,10838)|0);
   $468 = ($467|0)!=(0);
   if (!($468)) {
    HEAP32[$1>>2] = 33;
    break;
   }
   $469 = HEAP32[$3>>2]|0;
   $470 = (_strcmp($469,10850)|0);
   $471 = ($470|0)!=(0);
   if (!($471)) {
    HEAP32[$1>>2] = 34;
    break;
   }
   $472 = HEAP32[$3>>2]|0;
   $473 = (_strcmp($472,10874)|0);
   $474 = ($473|0)!=(0);
   if (!($474)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $475 = HEAP32[$3>>2]|0;
   $476 = (_strcmp($475,10887)|0);
   $477 = ($476|0)!=(0);
   if (!($477)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $478 = HEAP32[$3>>2]|0;
   $479 = (_strcmp($478,10901)|0);
   $480 = ($479|0)!=(0);
   if (!($480)) {
    HEAP32[$1>>2] = 35;
    break;
   }
   $481 = HEAP32[$3>>2]|0;
   $482 = (_strcmp($481,10923)|0);
   $483 = ($482|0)!=(0);
   if (!($483)) {
    HEAP32[$1>>2] = 36;
    break;
   }
   $484 = HEAP32[$3>>2]|0;
   $485 = (_strcmp($484,10930)|0);
   $486 = ($485|0)!=(0);
   if (!($486)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $487 = HEAP32[$3>>2]|0;
   $488 = (_strcmp($487,10946)|0);
   $489 = ($488|0)!=(0);
   if (!($489)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $490 = HEAP32[$3>>2]|0;
   $491 = (_strcmp($490,10963)|0);
   $492 = ($491|0)!=(0);
   if (!($492)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $493 = HEAP32[$3>>2]|0;
   $494 = (_strcmp($493,10980)|0);
   $495 = ($494|0)!=(0);
   if (!($495)) {
    HEAP32[$1>>2] = 28;
    break;
   }
   $496 = HEAP32[$3>>2]|0;
   $497 = (_strcmp($496,10996)|0);
   $498 = ($497|0)!=(0);
   if (!($498)) {
    HEAP32[$1>>2] = 1;
    break;
   }
   $499 = HEAP32[$3>>2]|0;
   $500 = (_strcmp($499,11012)|0);
   $501 = ($500|0)!=(0);
   if (!($501)) {
    HEAP32[$1>>2] = 4;
    break;
   }
   $502 = HEAP32[$3>>2]|0;
   $503 = (_strcmp($502,11029)|0);
   $504 = ($503|0)!=(0);
   if (!($504)) {
    HEAP32[$1>>2] = 29;
    break;
   }
   $505 = HEAP32[$3>>2]|0;
   $506 = (_strcmp($505,11043)|0);
   $507 = ($506|0)!=(0);
   if (!($507)) {
    HEAP32[$1>>2] = 30;
    break;
   }
   $508 = HEAP32[$3>>2]|0;
   $509 = (_strcmp($508,11055)|0);
   $510 = ($509|0)!=(0);
   if (!($510)) {
    HEAP32[$1>>2] = 22;
    break;
   }
   $511 = HEAP32[$3>>2]|0;
   $512 = (_strcmp($511,11066)|0);
   $513 = ($512|0)!=(0);
   if (!($513)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $514 = HEAP32[$3>>2]|0;
   $515 = (_strcmp($514,11079)|0);
   $516 = ($515|0)!=(0);
   if (!($516)) {
    HEAP32[$1>>2] = 23;
    break;
   }
   $517 = HEAP32[$3>>2]|0;
   $518 = (_strcmp($517,11089)|0);
   $519 = ($518|0)!=(0);
   if (!($519)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $520 = HEAP32[$3>>2]|0;
   $521 = (_strcmp($520,11106)|0);
   $522 = ($521|0)!=(0);
   if (!($522)) {
    HEAP32[$1>>2] = 24;
    break;
   }
   $523 = HEAP32[$3>>2]|0;
   $524 = (_strcmp($523,11118)|0);
   $525 = ($524|0)!=(0);
   if (!($525)) {
    HEAP32[$1>>2] = 25;
    break;
   }
   $526 = HEAP32[$3>>2]|0;
   $527 = (_strcmp($526,11140)|0);
   $528 = ($527|0)!=(0);
   if (!($528)) {
    HEAP32[$1>>2] = 26;
    break;
   }
   $529 = HEAP32[$3>>2]|0;
   $530 = (_strcmp($529,11160)|0);
   $531 = ($530|0)!=(0);
   if (!($531)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $532 = HEAP32[$3>>2]|0;
   $533 = (_strcmp($532,11173)|0);
   $534 = ($533|0)!=(0);
   if (!($534)) {
    HEAP32[$1>>2] = 27;
    break;
   }
   $535 = HEAP32[$3>>2]|0;
   $536 = (_strcmp($535,11195)|0);
   $537 = ($536|0)!=(0);
   if (!($537)) {
    HEAP32[$1>>2] = 28;
    break;
   }
   $538 = HEAP32[$3>>2]|0;
   $539 = (_strcmp($538,11215)|0);
   $540 = ($539|0)!=(0);
   if (!($540)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $541 = HEAP32[$3>>2]|0;
   $542 = (_strcmp($541,11232)|0);
   $543 = ($542|0)!=(0);
   if (!($543)) {
    HEAP32[$1>>2] = 2;
    break;
   }
   $544 = HEAP32[$3>>2]|0;
   $545 = (_strcmp($544,11249)|0);
   $546 = ($545|0)!=(0);
   if (!($546)) {
    HEAP32[$1>>2] = 3;
    break;
   }
   $547 = HEAP32[$3>>2]|0;
   $548 = (_strcmp($547,11269)|0);
   $549 = ($548|0)!=(0);
   if ($549) {
    $550 = HEAP32[$2>>2]|0;
    $551 = HEAP32[$3>>2]|0;
    $552 = _emscripten_asm_const_iii(0, ($550|0), ($551|0))|0;
    HEAP32[$1>>2] = 0;
    break;
   } else {
    HEAP32[$1>>2] = 37;
    break;
   }
  } else {
   HEAP32[$1>>2] = 5;
  }
 } while(0);
 $553 = HEAP32[$1>>2]|0;
 STACKTOP = sp;return ($553|0);
}
function _emscripten_get_global_libc() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 return (16356|0);
}
function ___stdio_close($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $vararg_buffer = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $vararg_buffer = sp;
 $1 = ((($0)) + 60|0);
 $2 = HEAP32[$1>>2]|0;
 $3 = (_dummy_738($2)|0);
 HEAP32[$vararg_buffer>>2] = $3;
 $4 = (___syscall6(6,($vararg_buffer|0))|0);
 $5 = (___syscall_ret($4)|0);
 STACKTOP = sp;return ($5|0);
}
function ___stdio_write($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$0 = 0, $$04756 = 0, $$04855 = 0, $$04954 = 0, $$051 = 0, $$1 = 0, $$150 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0;
 var $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0;
 var $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_buffer3 = 0, $vararg_ptr1 = 0, $vararg_ptr2 = 0, $vararg_ptr6 = 0;
 var $vararg_ptr7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 48|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(48|0);
 $vararg_buffer3 = sp + 16|0;
 $vararg_buffer = sp;
 $3 = sp + 32|0;
 $4 = ((($0)) + 28|0);
 $5 = HEAP32[$4>>2]|0;
 HEAP32[$3>>2] = $5;
 $6 = ((($3)) + 4|0);
 $7 = ((($0)) + 20|0);
 $8 = HEAP32[$7>>2]|0;
 $9 = (($8) - ($5))|0;
 HEAP32[$6>>2] = $9;
 $10 = ((($3)) + 8|0);
 HEAP32[$10>>2] = $1;
 $11 = ((($3)) + 12|0);
 HEAP32[$11>>2] = $2;
 $12 = (($9) + ($2))|0;
 $13 = ((($0)) + 60|0);
 $14 = HEAP32[$13>>2]|0;
 $15 = $3;
 HEAP32[$vararg_buffer>>2] = $14;
 $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
 HEAP32[$vararg_ptr1>>2] = $15;
 $vararg_ptr2 = ((($vararg_buffer)) + 8|0);
 HEAP32[$vararg_ptr2>>2] = 2;
 $16 = (___syscall146(146,($vararg_buffer|0))|0);
 $17 = (___syscall_ret($16)|0);
 $18 = ($12|0)==($17|0);
 L1: do {
  if ($18) {
   label = 3;
  } else {
   $$04756 = 2;$$04855 = $12;$$04954 = $3;$26 = $17;
   while(1) {
    $25 = ($26|0)<(0);
    if ($25) {
     break;
    }
    $34 = (($$04855) - ($26))|0;
    $35 = ((($$04954)) + 4|0);
    $36 = HEAP32[$35>>2]|0;
    $37 = ($26>>>0)>($36>>>0);
    $38 = ((($$04954)) + 8|0);
    $$150 = $37 ? $38 : $$04954;
    $39 = $37 << 31 >> 31;
    $$1 = (($39) + ($$04756))|0;
    $40 = $37 ? $36 : 0;
    $$0 = (($26) - ($40))|0;
    $41 = HEAP32[$$150>>2]|0;
    $42 = (($41) + ($$0)|0);
    HEAP32[$$150>>2] = $42;
    $43 = ((($$150)) + 4|0);
    $44 = HEAP32[$43>>2]|0;
    $45 = (($44) - ($$0))|0;
    HEAP32[$43>>2] = $45;
    $46 = HEAP32[$13>>2]|0;
    $47 = $$150;
    HEAP32[$vararg_buffer3>>2] = $46;
    $vararg_ptr6 = ((($vararg_buffer3)) + 4|0);
    HEAP32[$vararg_ptr6>>2] = $47;
    $vararg_ptr7 = ((($vararg_buffer3)) + 8|0);
    HEAP32[$vararg_ptr7>>2] = $$1;
    $48 = (___syscall146(146,($vararg_buffer3|0))|0);
    $49 = (___syscall_ret($48)|0);
    $50 = ($34|0)==($49|0);
    if ($50) {
     label = 3;
     break L1;
    } else {
     $$04756 = $$1;$$04855 = $34;$$04954 = $$150;$26 = $49;
    }
   }
   $27 = ((($0)) + 16|0);
   HEAP32[$27>>2] = 0;
   HEAP32[$4>>2] = 0;
   HEAP32[$7>>2] = 0;
   $28 = HEAP32[$0>>2]|0;
   $29 = $28 | 32;
   HEAP32[$0>>2] = $29;
   $30 = ($$04756|0)==(2);
   if ($30) {
    $$051 = 0;
   } else {
    $31 = ((($$04954)) + 4|0);
    $32 = HEAP32[$31>>2]|0;
    $33 = (($2) - ($32))|0;
    $$051 = $33;
   }
  }
 } while(0);
 if ((label|0) == 3) {
  $19 = ((($0)) + 44|0);
  $20 = HEAP32[$19>>2]|0;
  $21 = ((($0)) + 48|0);
  $22 = HEAP32[$21>>2]|0;
  $23 = (($20) + ($22)|0);
  $24 = ((($0)) + 16|0);
  HEAP32[$24>>2] = $23;
  HEAP32[$4>>2] = $20;
  HEAP32[$7>>2] = $20;
  $$051 = $2;
 }
 STACKTOP = sp;return ($$051|0);
}
function ___stdio_seek($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$pre = 0, $10 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_ptr1 = 0, $vararg_ptr2 = 0, $vararg_ptr3 = 0, $vararg_ptr4 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $vararg_buffer = sp;
 $3 = sp + 20|0;
 $4 = ((($0)) + 60|0);
 $5 = HEAP32[$4>>2]|0;
 $6 = $3;
 HEAP32[$vararg_buffer>>2] = $5;
 $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
 HEAP32[$vararg_ptr1>>2] = 0;
 $vararg_ptr2 = ((($vararg_buffer)) + 8|0);
 HEAP32[$vararg_ptr2>>2] = $1;
 $vararg_ptr3 = ((($vararg_buffer)) + 12|0);
 HEAP32[$vararg_ptr3>>2] = $6;
 $vararg_ptr4 = ((($vararg_buffer)) + 16|0);
 HEAP32[$vararg_ptr4>>2] = $2;
 $7 = (___syscall140(140,($vararg_buffer|0))|0);
 $8 = (___syscall_ret($7)|0);
 $9 = ($8|0)<(0);
 if ($9) {
  HEAP32[$3>>2] = -1;
  $10 = -1;
 } else {
  $$pre = HEAP32[$3>>2]|0;
  $10 = $$pre;
 }
 STACKTOP = sp;return ($10|0);
}
function ___syscall_ret($0) {
 $0 = $0|0;
 var $$0 = 0, $1 = 0, $2 = 0, $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ($0>>>0)>(4294963200);
 if ($1) {
  $2 = (0 - ($0))|0;
  $3 = (___errno_location()|0);
  HEAP32[$3>>2] = $2;
  $$0 = -1;
 } else {
  $$0 = $0;
 }
 return ($$0|0);
}
function ___errno_location() {
 var $0 = 0, $1 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (___pthread_self_108()|0);
 $1 = ((($0)) + 64|0);
 return ($1|0);
}
function ___pthread_self_108() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (_pthread_self()|0);
 return ($0|0);
}
function _pthread_self() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 return (2992|0);
}
function _dummy_738($0) {
 $0 = $0|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 return ($0|0);
}
function ___stdout_write($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $vararg_buffer = 0, $vararg_ptr1 = 0, $vararg_ptr2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $vararg_buffer = sp;
 $3 = sp + 16|0;
 $4 = ((($0)) + 36|0);
 HEAP32[$4>>2] = 9;
 $5 = HEAP32[$0>>2]|0;
 $6 = $5 & 64;
 $7 = ($6|0)==(0);
 if ($7) {
  $8 = ((($0)) + 60|0);
  $9 = HEAP32[$8>>2]|0;
  $10 = $3;
  HEAP32[$vararg_buffer>>2] = $9;
  $vararg_ptr1 = ((($vararg_buffer)) + 4|0);
  HEAP32[$vararg_ptr1>>2] = 21523;
  $vararg_ptr2 = ((($vararg_buffer)) + 8|0);
  HEAP32[$vararg_ptr2>>2] = $10;
  $11 = (___syscall54(54,($vararg_buffer|0))|0);
  $12 = ($11|0)==(0);
  if (!($12)) {
   $13 = ((($0)) + 75|0);
   HEAP8[$13>>0] = -1;
  }
 }
 $14 = (___stdio_write($0,$1,$2)|0);
 STACKTOP = sp;return ($14|0);
}
function _strcmp($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$011 = 0, $$0710 = 0, $$lcssa = 0, $$lcssa8 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond9 = 0, label = 0;
 var sp = 0;
 sp = STACKTOP;
 $2 = HEAP8[$0>>0]|0;
 $3 = HEAP8[$1>>0]|0;
 $4 = ($2<<24>>24)!=($3<<24>>24);
 $5 = ($2<<24>>24)==(0);
 $or$cond9 = $5 | $4;
 if ($or$cond9) {
  $$lcssa = $3;$$lcssa8 = $2;
 } else {
  $$011 = $1;$$0710 = $0;
  while(1) {
   $6 = ((($$0710)) + 1|0);
   $7 = ((($$011)) + 1|0);
   $8 = HEAP8[$6>>0]|0;
   $9 = HEAP8[$7>>0]|0;
   $10 = ($8<<24>>24)!=($9<<24>>24);
   $11 = ($8<<24>>24)==(0);
   $or$cond = $11 | $10;
   if ($or$cond) {
    $$lcssa = $9;$$lcssa8 = $8;
    break;
   } else {
    $$011 = $7;$$0710 = $6;
   }
  }
 }
 $12 = $$lcssa8&255;
 $13 = $$lcssa&255;
 $14 = (($12) - ($13))|0;
 return ($14|0);
}
function _memcmp($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$01318 = 0, $$01417 = 0, $$019 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($2|0)==(0);
 L1: do {
  if ($3) {
   $14 = 0;
  } else {
   $$01318 = $0;$$01417 = $2;$$019 = $1;
   while(1) {
    $4 = HEAP8[$$01318>>0]|0;
    $5 = HEAP8[$$019>>0]|0;
    $6 = ($4<<24>>24)==($5<<24>>24);
    if (!($6)) {
     break;
    }
    $7 = (($$01417) + -1)|0;
    $8 = ((($$01318)) + 1|0);
    $9 = ((($$019)) + 1|0);
    $10 = ($7|0)==(0);
    if ($10) {
     $14 = 0;
     break L1;
    } else {
     $$01318 = $8;$$01417 = $7;$$019 = $9;
    }
   }
   $11 = $4&255;
   $12 = $5&255;
   $13 = (($11) - ($12))|0;
   $14 = $13;
  }
 } while(0);
 return ($14|0);
}
function _vsprintf($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = (_vsnprintf($0,2147483647,$1,$2)|0);
 return ($3|0);
}
function _vsnprintf($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $$$015 = 0, $$0 = 0, $$014 = 0, $$015 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0;
 var $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, dest = 0, label = 0, sp = 0, src = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 128|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(128|0);
 $4 = sp + 124|0;
 $5 = sp;
 dest=$5; src=3368; stop=dest+124|0; do { HEAP32[dest>>2]=HEAP32[src>>2]|0; dest=dest+4|0; src=src+4|0; } while ((dest|0) < (stop|0));
 $6 = (($1) + -1)|0;
 $7 = ($6>>>0)>(2147483646);
 if ($7) {
  $8 = ($1|0)==(0);
  if ($8) {
   $$014 = $4;$$015 = 1;
   label = 4;
  } else {
   $9 = (___errno_location()|0);
   HEAP32[$9>>2] = 75;
   $$0 = -1;
  }
 } else {
  $$014 = $0;$$015 = $1;
  label = 4;
 }
 if ((label|0) == 4) {
  $10 = $$014;
  $11 = (-2 - ($10))|0;
  $12 = ($$015>>>0)>($11>>>0);
  $$$015 = $12 ? $11 : $$015;
  $13 = ((($5)) + 48|0);
  HEAP32[$13>>2] = $$$015;
  $14 = ((($5)) + 20|0);
  HEAP32[$14>>2] = $$014;
  $15 = ((($5)) + 44|0);
  HEAP32[$15>>2] = $$014;
  $16 = (($$014) + ($$$015)|0);
  $17 = ((($5)) + 16|0);
  HEAP32[$17>>2] = $16;
  $18 = ((($5)) + 28|0);
  HEAP32[$18>>2] = $16;
  $19 = (_vfprintf($5,$2,$3)|0);
  $20 = ($$$015|0)==(0);
  if ($20) {
   $$0 = $19;
  } else {
   $21 = HEAP32[$14>>2]|0;
   $22 = HEAP32[$17>>2]|0;
   $23 = ($21|0)==($22|0);
   $24 = $23 << 31 >> 31;
   $25 = (($21) + ($24)|0);
   HEAP8[$25>>0] = 0;
   $$0 = $19;
  }
 }
 STACKTOP = sp;return ($$0|0);
}
function _vfprintf($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$ = 0, $$0 = 0, $$1 = 0, $$1$ = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0;
 var $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $5 = 0, $6 = 0, $7 = 0;
 var $8 = 0, $9 = 0, $vacopy_currentptr = 0, dest = 0, label = 0, sp = 0, stop = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 224|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(224|0);
 $3 = sp + 120|0;
 $4 = sp + 80|0;
 $5 = sp;
 $6 = sp + 136|0;
 dest=$4; stop=dest+40|0; do { HEAP32[dest>>2]=0|0; dest=dest+4|0; } while ((dest|0) < (stop|0));
 $vacopy_currentptr = HEAP32[$2>>2]|0;
 HEAP32[$3>>2] = $vacopy_currentptr;
 $7 = (_printf_core(0,$1,$3,$5,$4)|0);
 $8 = ($7|0)<(0);
 if ($8) {
  $$0 = -1;
 } else {
  $9 = ((($0)) + 76|0);
  $10 = HEAP32[$9>>2]|0;
  $11 = ($10|0)>(-1);
  if ($11) {
   $12 = (___lockfile($0)|0);
   $40 = $12;
  } else {
   $40 = 0;
  }
  $13 = HEAP32[$0>>2]|0;
  $14 = $13 & 32;
  $15 = ((($0)) + 74|0);
  $16 = HEAP8[$15>>0]|0;
  $17 = ($16<<24>>24)<(1);
  if ($17) {
   $18 = $13 & -33;
   HEAP32[$0>>2] = $18;
  }
  $19 = ((($0)) + 48|0);
  $20 = HEAP32[$19>>2]|0;
  $21 = ($20|0)==(0);
  if ($21) {
   $23 = ((($0)) + 44|0);
   $24 = HEAP32[$23>>2]|0;
   HEAP32[$23>>2] = $6;
   $25 = ((($0)) + 28|0);
   HEAP32[$25>>2] = $6;
   $26 = ((($0)) + 20|0);
   HEAP32[$26>>2] = $6;
   HEAP32[$19>>2] = 80;
   $27 = ((($6)) + 80|0);
   $28 = ((($0)) + 16|0);
   HEAP32[$28>>2] = $27;
   $29 = (_printf_core($0,$1,$3,$5,$4)|0);
   $30 = ($24|0)==(0|0);
   if ($30) {
    $$1 = $29;
   } else {
    $31 = ((($0)) + 36|0);
    $32 = HEAP32[$31>>2]|0;
    (FUNCTION_TABLE_iiii[$32 & 15]($0,0,0)|0);
    $33 = HEAP32[$26>>2]|0;
    $34 = ($33|0)==(0|0);
    $$ = $34 ? -1 : $29;
    HEAP32[$23>>2] = $24;
    HEAP32[$19>>2] = 0;
    HEAP32[$28>>2] = 0;
    HEAP32[$25>>2] = 0;
    HEAP32[$26>>2] = 0;
    $$1 = $$;
   }
  } else {
   $22 = (_printf_core($0,$1,$3,$5,$4)|0);
   $$1 = $22;
  }
  $35 = HEAP32[$0>>2]|0;
  $36 = $35 & 32;
  $37 = ($36|0)==(0);
  $$1$ = $37 ? $$1 : -1;
  $38 = $35 | $14;
  HEAP32[$0>>2] = $38;
  $39 = ($40|0)==(0);
  if (!($39)) {
   ___unlockfile($0);
  }
  $$0 = $$1$;
 }
 STACKTOP = sp;return ($$0|0);
}
function _printf_core($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $$ = 0, $$$ = 0, $$$0259 = 0, $$$0262 = 0, $$$0269 = 0, $$$4266 = 0, $$$5 = 0, $$0 = 0, $$0228 = 0, $$0228$ = 0, $$0229322 = 0, $$0232 = 0, $$0235 = 0, $$0237 = 0, $$0240$lcssa = 0, $$0240$lcssa357 = 0, $$0240321 = 0, $$0243 = 0, $$0247 = 0, $$0249$lcssa = 0;
 var $$0249306 = 0, $$0252 = 0, $$0253 = 0, $$0254 = 0, $$0254$$0254$ = 0, $$0259 = 0, $$0262$lcssa = 0, $$0262311 = 0, $$0269 = 0, $$0269$phi = 0, $$1 = 0, $$1230333 = 0, $$1233 = 0, $$1236 = 0, $$1238 = 0, $$1241332 = 0, $$1244320 = 0, $$1248 = 0, $$1250 = 0, $$1255 = 0;
 var $$1260 = 0, $$1263 = 0, $$1263$ = 0, $$1270 = 0, $$2 = 0, $$2234 = 0, $$2239 = 0, $$2242305 = 0, $$2245 = 0, $$2251 = 0, $$2256 = 0, $$2256$ = 0, $$2256$$$2256 = 0, $$2261 = 0, $$2271 = 0, $$284$ = 0, $$289 = 0, $$290 = 0, $$3257 = 0, $$3265 = 0;
 var $$3272 = 0, $$3303 = 0, $$377 = 0, $$4258355 = 0, $$4266 = 0, $$5 = 0, $$6268 = 0, $$lcssa295 = 0, $$pre = 0, $$pre346 = 0, $$pre347 = 0, $$pre347$pre = 0, $$pre349 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0;
 var $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0;
 var $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0;
 var $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0, $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0, $16 = 0;
 var $160 = 0, $161 = 0, $162 = 0, $163 = 0, $164 = 0, $165 = 0, $166 = 0, $167 = 0, $168 = 0, $169 = 0, $17 = 0, $170 = 0, $171 = 0, $172 = 0, $173 = 0, $174 = 0, $175 = 0, $176 = 0, $177 = 0, $178 = 0;
 var $179 = 0, $18 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0, $184 = 0, $185 = 0, $186 = 0, $187 = 0, $188 = 0, $189 = 0, $19 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0;
 var $197 = 0, $198 = 0, $199 = 0, $20 = 0, $200 = 0, $201 = 0, $202 = 0, $203 = 0, $204 = 0, $205 = 0, $206 = 0, $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0, $212 = 0, $213 = 0, $214 = 0;
 var $215 = 0, $216 = 0, $217 = 0, $218 = 0, $219 = 0, $22 = 0, $220 = 0, $221 = 0, $222 = 0, $223 = 0, $224 = 0, $225 = 0, $226 = 0, $227 = 0, $228 = 0, $229 = 0, $23 = 0, $230 = 0, $231 = 0, $232 = 0;
 var $233 = 0, $234 = 0, $235 = 0, $236 = 0, $237 = 0, $238 = 0, $239 = 0, $24 = 0, $240 = 0, $241 = 0, $242 = 0, $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0, $249 = 0, $25 = 0, $250 = 0;
 var $251 = 0, $252 = 0, $253 = 0, $254 = 0, $255 = 0, $256 = 0, $257 = 0, $258 = 0, $259 = 0, $26 = 0, $260 = 0, $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0, $267 = 0, $268 = 0, $269 = 0;
 var $27 = 0, $270 = 0, $271 = 0, $272 = 0, $273 = 0, $274 = 0, $275 = 0, $276 = 0, $277 = 0, $278 = 0, $279 = 0, $28 = 0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0, $285 = 0, $286 = 0, $287 = 0;
 var $288 = 0, $289 = 0, $29 = 0, $290 = 0, $291 = 0, $292 = 0, $293 = 0, $294 = 0, $295 = 0, $296 = 0, $297 = 0, $298 = 0, $299 = 0, $30 = 0, $300 = 0, $301 = 0, $302 = 0, $303 = 0, $304 = 0, $305 = 0;
 var $306 = 0.0, $307 = 0, $308 = 0, $309 = 0, $31 = 0, $310 = 0, $311 = 0, $312 = 0, $313 = 0, $314 = 0, $315 = 0, $316 = 0, $317 = 0, $318 = 0, $319 = 0, $32 = 0, $320 = 0, $321 = 0, $322 = 0, $323 = 0;
 var $324 = 0, $325 = 0, $326 = 0, $327 = 0, $328 = 0, $329 = 0, $33 = 0, $330 = 0, $331 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0;
 var $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0;
 var $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0;
 var $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0;
 var $arglist_current = 0, $arglist_current2 = 0, $arglist_next = 0, $arglist_next3 = 0, $expanded = 0, $expanded10 = 0, $expanded11 = 0, $expanded13 = 0, $expanded14 = 0, $expanded15 = 0, $expanded4 = 0, $expanded6 = 0, $expanded7 = 0, $expanded8 = 0, $isdigit = 0, $isdigit275 = 0, $isdigit277 = 0, $isdigittmp = 0, $isdigittmp$ = 0, $isdigittmp274 = 0;
 var $isdigittmp276 = 0, $narrow = 0, $or$cond = 0, $or$cond281 = 0, $or$cond283 = 0, $or$cond286 = 0, $storemerge = 0, $storemerge273310 = 0, $storemerge278 = 0, $trunc = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 64|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(64|0);
 $5 = sp + 16|0;
 $6 = sp;
 $7 = sp + 24|0;
 $8 = sp + 8|0;
 $9 = sp + 20|0;
 HEAP32[$5>>2] = $1;
 $10 = ($0|0)!=(0|0);
 $11 = ((($7)) + 40|0);
 $12 = $11;
 $13 = ((($7)) + 39|0);
 $14 = ((($8)) + 4|0);
 $$0243 = 0;$$0247 = 0;$$0269 = 0;$21 = $1;
 L1: while(1) {
  $15 = ($$0247|0)>(-1);
  do {
   if ($15) {
    $16 = (2147483647 - ($$0247))|0;
    $17 = ($$0243|0)>($16|0);
    if ($17) {
     $18 = (___errno_location()|0);
     HEAP32[$18>>2] = 75;
     $$1248 = -1;
     break;
    } else {
     $19 = (($$0243) + ($$0247))|0;
     $$1248 = $19;
     break;
    }
   } else {
    $$1248 = $$0247;
   }
  } while(0);
  $20 = HEAP8[$21>>0]|0;
  $22 = ($20<<24>>24)==(0);
  if ($22) {
   label = 87;
   break;
  } else {
   $23 = $20;$25 = $21;
  }
  L9: while(1) {
   switch ($23<<24>>24) {
   case 37:  {
    $$0249306 = $25;$27 = $25;
    label = 9;
    break L9;
    break;
   }
   case 0:  {
    $$0249$lcssa = $25;$39 = $25;
    break L9;
    break;
   }
   default: {
   }
   }
   $24 = ((($25)) + 1|0);
   HEAP32[$5>>2] = $24;
   $$pre = HEAP8[$24>>0]|0;
   $23 = $$pre;$25 = $24;
  }
  L12: do {
   if ((label|0) == 9) {
    while(1) {
     label = 0;
     $26 = ((($27)) + 1|0);
     $28 = HEAP8[$26>>0]|0;
     $29 = ($28<<24>>24)==(37);
     if (!($29)) {
      $$0249$lcssa = $$0249306;$39 = $27;
      break L12;
     }
     $30 = ((($$0249306)) + 1|0);
     $31 = ((($27)) + 2|0);
     HEAP32[$5>>2] = $31;
     $32 = HEAP8[$31>>0]|0;
     $33 = ($32<<24>>24)==(37);
     if ($33) {
      $$0249306 = $30;$27 = $31;
      label = 9;
     } else {
      $$0249$lcssa = $30;$39 = $31;
      break;
     }
    }
   }
  } while(0);
  $34 = $$0249$lcssa;
  $35 = $21;
  $36 = (($34) - ($35))|0;
  if ($10) {
   _out($0,$21,$36);
  }
  $37 = ($36|0)==(0);
  if (!($37)) {
   $$0269$phi = $$0269;$$0243 = $36;$$0247 = $$1248;$21 = $39;$$0269 = $$0269$phi;
   continue;
  }
  $38 = ((($39)) + 1|0);
  $40 = HEAP8[$38>>0]|0;
  $41 = $40 << 24 >> 24;
  $isdigittmp = (($41) + -48)|0;
  $isdigit = ($isdigittmp>>>0)<(10);
  if ($isdigit) {
   $42 = ((($39)) + 2|0);
   $43 = HEAP8[$42>>0]|0;
   $44 = ($43<<24>>24)==(36);
   $45 = ((($39)) + 3|0);
   $$377 = $44 ? $45 : $38;
   $$$0269 = $44 ? 1 : $$0269;
   $isdigittmp$ = $44 ? $isdigittmp : -1;
   $$0253 = $isdigittmp$;$$1270 = $$$0269;$storemerge = $$377;
  } else {
   $$0253 = -1;$$1270 = $$0269;$storemerge = $38;
  }
  HEAP32[$5>>2] = $storemerge;
  $46 = HEAP8[$storemerge>>0]|0;
  $47 = $46 << 24 >> 24;
  $48 = (($47) + -32)|0;
  $49 = ($48>>>0)<(32);
  L24: do {
   if ($49) {
    $$0262311 = 0;$329 = $46;$51 = $48;$storemerge273310 = $storemerge;
    while(1) {
     $50 = 1 << $51;
     $52 = $50 & 75913;
     $53 = ($52|0)==(0);
     if ($53) {
      $$0262$lcssa = $$0262311;$$lcssa295 = $329;$62 = $storemerge273310;
      break L24;
     }
     $54 = $50 | $$0262311;
     $55 = ((($storemerge273310)) + 1|0);
     HEAP32[$5>>2] = $55;
     $56 = HEAP8[$55>>0]|0;
     $57 = $56 << 24 >> 24;
     $58 = (($57) + -32)|0;
     $59 = ($58>>>0)<(32);
     if ($59) {
      $$0262311 = $54;$329 = $56;$51 = $58;$storemerge273310 = $55;
     } else {
      $$0262$lcssa = $54;$$lcssa295 = $56;$62 = $55;
      break;
     }
    }
   } else {
    $$0262$lcssa = 0;$$lcssa295 = $46;$62 = $storemerge;
   }
  } while(0);
  $60 = ($$lcssa295<<24>>24)==(42);
  if ($60) {
   $61 = ((($62)) + 1|0);
   $63 = HEAP8[$61>>0]|0;
   $64 = $63 << 24 >> 24;
   $isdigittmp276 = (($64) + -48)|0;
   $isdigit277 = ($isdigittmp276>>>0)<(10);
   if ($isdigit277) {
    $65 = ((($62)) + 2|0);
    $66 = HEAP8[$65>>0]|0;
    $67 = ($66<<24>>24)==(36);
    if ($67) {
     $68 = (($4) + ($isdigittmp276<<2)|0);
     HEAP32[$68>>2] = 10;
     $69 = HEAP8[$61>>0]|0;
     $70 = $69 << 24 >> 24;
     $71 = (($70) + -48)|0;
     $72 = (($3) + ($71<<3)|0);
     $73 = $72;
     $74 = $73;
     $75 = HEAP32[$74>>2]|0;
     $76 = (($73) + 4)|0;
     $77 = $76;
     $78 = HEAP32[$77>>2]|0;
     $79 = ((($62)) + 3|0);
     $$0259 = $75;$$2271 = 1;$storemerge278 = $79;
    } else {
     label = 23;
    }
   } else {
    label = 23;
   }
   if ((label|0) == 23) {
    label = 0;
    $80 = ($$1270|0)==(0);
    if (!($80)) {
     $$0 = -1;
     break;
    }
    if ($10) {
     $arglist_current = HEAP32[$2>>2]|0;
     $81 = $arglist_current;
     $82 = ((0) + 4|0);
     $expanded4 = $82;
     $expanded = (($expanded4) - 1)|0;
     $83 = (($81) + ($expanded))|0;
     $84 = ((0) + 4|0);
     $expanded8 = $84;
     $expanded7 = (($expanded8) - 1)|0;
     $expanded6 = $expanded7 ^ -1;
     $85 = $83 & $expanded6;
     $86 = $85;
     $87 = HEAP32[$86>>2]|0;
     $arglist_next = ((($86)) + 4|0);
     HEAP32[$2>>2] = $arglist_next;
     $$0259 = $87;$$2271 = 0;$storemerge278 = $61;
    } else {
     $$0259 = 0;$$2271 = 0;$storemerge278 = $61;
    }
   }
   HEAP32[$5>>2] = $storemerge278;
   $88 = ($$0259|0)<(0);
   $89 = $$0262$lcssa | 8192;
   $90 = (0 - ($$0259))|0;
   $$$0262 = $88 ? $89 : $$0262$lcssa;
   $$$0259 = $88 ? $90 : $$0259;
   $$1260 = $$$0259;$$1263 = $$$0262;$$3272 = $$2271;$94 = $storemerge278;
  } else {
   $91 = (_getint($5)|0);
   $92 = ($91|0)<(0);
   if ($92) {
    $$0 = -1;
    break;
   }
   $$pre346 = HEAP32[$5>>2]|0;
   $$1260 = $91;$$1263 = $$0262$lcssa;$$3272 = $$1270;$94 = $$pre346;
  }
  $93 = HEAP8[$94>>0]|0;
  $95 = ($93<<24>>24)==(46);
  do {
   if ($95) {
    $96 = ((($94)) + 1|0);
    $97 = HEAP8[$96>>0]|0;
    $98 = ($97<<24>>24)==(42);
    if (!($98)) {
     $125 = ((($94)) + 1|0);
     HEAP32[$5>>2] = $125;
     $126 = (_getint($5)|0);
     $$pre347$pre = HEAP32[$5>>2]|0;
     $$0254 = $126;$$pre347 = $$pre347$pre;
     break;
    }
    $99 = ((($94)) + 2|0);
    $100 = HEAP8[$99>>0]|0;
    $101 = $100 << 24 >> 24;
    $isdigittmp274 = (($101) + -48)|0;
    $isdigit275 = ($isdigittmp274>>>0)<(10);
    if ($isdigit275) {
     $102 = ((($94)) + 3|0);
     $103 = HEAP8[$102>>0]|0;
     $104 = ($103<<24>>24)==(36);
     if ($104) {
      $105 = (($4) + ($isdigittmp274<<2)|0);
      HEAP32[$105>>2] = 10;
      $106 = HEAP8[$99>>0]|0;
      $107 = $106 << 24 >> 24;
      $108 = (($107) + -48)|0;
      $109 = (($3) + ($108<<3)|0);
      $110 = $109;
      $111 = $110;
      $112 = HEAP32[$111>>2]|0;
      $113 = (($110) + 4)|0;
      $114 = $113;
      $115 = HEAP32[$114>>2]|0;
      $116 = ((($94)) + 4|0);
      HEAP32[$5>>2] = $116;
      $$0254 = $112;$$pre347 = $116;
      break;
     }
    }
    $117 = ($$3272|0)==(0);
    if (!($117)) {
     $$0 = -1;
     break L1;
    }
    if ($10) {
     $arglist_current2 = HEAP32[$2>>2]|0;
     $118 = $arglist_current2;
     $119 = ((0) + 4|0);
     $expanded11 = $119;
     $expanded10 = (($expanded11) - 1)|0;
     $120 = (($118) + ($expanded10))|0;
     $121 = ((0) + 4|0);
     $expanded15 = $121;
     $expanded14 = (($expanded15) - 1)|0;
     $expanded13 = $expanded14 ^ -1;
     $122 = $120 & $expanded13;
     $123 = $122;
     $124 = HEAP32[$123>>2]|0;
     $arglist_next3 = ((($123)) + 4|0);
     HEAP32[$2>>2] = $arglist_next3;
     $330 = $124;
    } else {
     $330 = 0;
    }
    HEAP32[$5>>2] = $99;
    $$0254 = $330;$$pre347 = $99;
   } else {
    $$0254 = -1;$$pre347 = $94;
   }
  } while(0);
  $$0252 = 0;$128 = $$pre347;
  while(1) {
   $127 = HEAP8[$128>>0]|0;
   $129 = $127 << 24 >> 24;
   $130 = (($129) + -65)|0;
   $131 = ($130>>>0)>(57);
   if ($131) {
    $$0 = -1;
    break L1;
   }
   $132 = ((($128)) + 1|0);
   HEAP32[$5>>2] = $132;
   $133 = HEAP8[$128>>0]|0;
   $134 = $133 << 24 >> 24;
   $135 = (($134) + -65)|0;
   $136 = ((11385 + (($$0252*58)|0)|0) + ($135)|0);
   $137 = HEAP8[$136>>0]|0;
   $138 = $137&255;
   $139 = (($138) + -1)|0;
   $140 = ($139>>>0)<(8);
   if ($140) {
    $$0252 = $138;$128 = $132;
   } else {
    break;
   }
  }
  $141 = ($137<<24>>24)==(0);
  if ($141) {
   $$0 = -1;
   break;
  }
  $142 = ($137<<24>>24)==(19);
  $143 = ($$0253|0)>(-1);
  do {
   if ($142) {
    if ($143) {
     $$0 = -1;
     break L1;
    } else {
     label = 49;
    }
   } else {
    if ($143) {
     $144 = (($4) + ($$0253<<2)|0);
     HEAP32[$144>>2] = $138;
     $145 = (($3) + ($$0253<<3)|0);
     $146 = $145;
     $147 = $146;
     $148 = HEAP32[$147>>2]|0;
     $149 = (($146) + 4)|0;
     $150 = $149;
     $151 = HEAP32[$150>>2]|0;
     $152 = $6;
     $153 = $152;
     HEAP32[$153>>2] = $148;
     $154 = (($152) + 4)|0;
     $155 = $154;
     HEAP32[$155>>2] = $151;
     label = 49;
     break;
    }
    if (!($10)) {
     $$0 = 0;
     break L1;
    }
    _pop_arg($6,$138,$2);
   }
  } while(0);
  if ((label|0) == 49) {
   label = 0;
   if (!($10)) {
    $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
    continue;
   }
  }
  $156 = HEAP8[$128>>0]|0;
  $157 = $156 << 24 >> 24;
  $158 = ($$0252|0)!=(0);
  $159 = $157 & 15;
  $160 = ($159|0)==(3);
  $or$cond281 = $158 & $160;
  $161 = $157 & -33;
  $$0235 = $or$cond281 ? $161 : $157;
  $162 = $$1263 & 8192;
  $163 = ($162|0)==(0);
  $164 = $$1263 & -65537;
  $$1263$ = $163 ? $$1263 : $164;
  L71: do {
   switch ($$0235|0) {
   case 110:  {
    $trunc = $$0252&255;
    switch ($trunc<<24>>24) {
    case 0:  {
     $171 = HEAP32[$6>>2]|0;
     HEAP32[$171>>2] = $$1248;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 1:  {
     $172 = HEAP32[$6>>2]|0;
     HEAP32[$172>>2] = $$1248;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 2:  {
     $173 = ($$1248|0)<(0);
     $174 = $173 << 31 >> 31;
     $175 = HEAP32[$6>>2]|0;
     $176 = $175;
     $177 = $176;
     HEAP32[$177>>2] = $$1248;
     $178 = (($176) + 4)|0;
     $179 = $178;
     HEAP32[$179>>2] = $174;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 3:  {
     $180 = $$1248&65535;
     $181 = HEAP32[$6>>2]|0;
     HEAP16[$181>>1] = $180;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 4:  {
     $182 = $$1248&255;
     $183 = HEAP32[$6>>2]|0;
     HEAP8[$183>>0] = $182;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 6:  {
     $184 = HEAP32[$6>>2]|0;
     HEAP32[$184>>2] = $$1248;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    case 7:  {
     $185 = ($$1248|0)<(0);
     $186 = $185 << 31 >> 31;
     $187 = HEAP32[$6>>2]|0;
     $188 = $187;
     $189 = $188;
     HEAP32[$189>>2] = $$1248;
     $190 = (($188) + 4)|0;
     $191 = $190;
     HEAP32[$191>>2] = $186;
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
     break;
    }
    default: {
     $$0243 = 0;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
     continue L1;
    }
    }
    break;
   }
   case 112:  {
    $192 = ($$0254>>>0)>(8);
    $193 = $192 ? $$0254 : 8;
    $194 = $$1263$ | 8;
    $$1236 = 120;$$1255 = $193;$$3265 = $194;
    label = 61;
    break;
   }
   case 88: case 120:  {
    $$1236 = $$0235;$$1255 = $$0254;$$3265 = $$1263$;
    label = 61;
    break;
   }
   case 111:  {
    $210 = $6;
    $211 = $210;
    $212 = HEAP32[$211>>2]|0;
    $213 = (($210) + 4)|0;
    $214 = $213;
    $215 = HEAP32[$214>>2]|0;
    $216 = (_fmt_o($212,$215,$11)|0);
    $217 = $$1263$ & 8;
    $218 = ($217|0)==(0);
    $219 = $216;
    $220 = (($12) - ($219))|0;
    $221 = ($$0254|0)>($220|0);
    $222 = (($220) + 1)|0;
    $223 = $218 | $221;
    $$0254$$0254$ = $223 ? $$0254 : $222;
    $$0228 = $216;$$1233 = 0;$$1238 = 11849;$$2256 = $$0254$$0254$;$$4266 = $$1263$;$248 = $212;$250 = $215;
    label = 67;
    break;
   }
   case 105: case 100:  {
    $224 = $6;
    $225 = $224;
    $226 = HEAP32[$225>>2]|0;
    $227 = (($224) + 4)|0;
    $228 = $227;
    $229 = HEAP32[$228>>2]|0;
    $230 = ($229|0)<(0);
    if ($230) {
     $231 = (_i64Subtract(0,0,($226|0),($229|0))|0);
     $232 = tempRet0;
     $233 = $6;
     $234 = $233;
     HEAP32[$234>>2] = $231;
     $235 = (($233) + 4)|0;
     $236 = $235;
     HEAP32[$236>>2] = $232;
     $$0232 = 1;$$0237 = 11849;$242 = $231;$243 = $232;
     label = 66;
     break L71;
    } else {
     $237 = $$1263$ & 2048;
     $238 = ($237|0)==(0);
     $239 = $$1263$ & 1;
     $240 = ($239|0)==(0);
     $$ = $240 ? 11849 : (11851);
     $$$ = $238 ? $$ : (11850);
     $241 = $$1263$ & 2049;
     $narrow = ($241|0)!=(0);
     $$284$ = $narrow&1;
     $$0232 = $$284$;$$0237 = $$$;$242 = $226;$243 = $229;
     label = 66;
     break L71;
    }
    break;
   }
   case 117:  {
    $165 = $6;
    $166 = $165;
    $167 = HEAP32[$166>>2]|0;
    $168 = (($165) + 4)|0;
    $169 = $168;
    $170 = HEAP32[$169>>2]|0;
    $$0232 = 0;$$0237 = 11849;$242 = $167;$243 = $170;
    label = 66;
    break;
   }
   case 99:  {
    $259 = $6;
    $260 = $259;
    $261 = HEAP32[$260>>2]|0;
    $262 = (($259) + 4)|0;
    $263 = $262;
    $264 = HEAP32[$263>>2]|0;
    $265 = $261&255;
    HEAP8[$13>>0] = $265;
    $$2 = $13;$$2234 = 0;$$2239 = 11849;$$2251 = $11;$$5 = 1;$$6268 = $164;
    break;
   }
   case 109:  {
    $266 = (___errno_location()|0);
    $267 = HEAP32[$266>>2]|0;
    $268 = (_strerror($267)|0);
    $$1 = $268;
    label = 71;
    break;
   }
   case 115:  {
    $269 = HEAP32[$6>>2]|0;
    $270 = ($269|0)!=(0|0);
    $271 = $270 ? $269 : 11859;
    $$1 = $271;
    label = 71;
    break;
   }
   case 67:  {
    $278 = $6;
    $279 = $278;
    $280 = HEAP32[$279>>2]|0;
    $281 = (($278) + 4)|0;
    $282 = $281;
    $283 = HEAP32[$282>>2]|0;
    HEAP32[$8>>2] = $280;
    HEAP32[$14>>2] = 0;
    HEAP32[$6>>2] = $8;
    $$4258355 = -1;$331 = $8;
    label = 75;
    break;
   }
   case 83:  {
    $$pre349 = HEAP32[$6>>2]|0;
    $284 = ($$0254|0)==(0);
    if ($284) {
     _pad_674($0,32,$$1260,0,$$1263$);
     $$0240$lcssa357 = 0;
     label = 84;
    } else {
     $$4258355 = $$0254;$331 = $$pre349;
     label = 75;
    }
    break;
   }
   case 65: case 71: case 70: case 69: case 97: case 103: case 102: case 101:  {
    $306 = +HEAPF64[$6>>3];
    $307 = (_fmt_fp($0,$306,$$1260,$$0254,$$1263$,$$0235)|0);
    $$0243 = $307;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
    continue L1;
    break;
   }
   default: {
    $$2 = $21;$$2234 = 0;$$2239 = 11849;$$2251 = $11;$$5 = $$0254;$$6268 = $$1263$;
   }
   }
  } while(0);
  L95: do {
   if ((label|0) == 61) {
    label = 0;
    $195 = $6;
    $196 = $195;
    $197 = HEAP32[$196>>2]|0;
    $198 = (($195) + 4)|0;
    $199 = $198;
    $200 = HEAP32[$199>>2]|0;
    $201 = $$1236 & 32;
    $202 = (_fmt_x($197,$200,$11,$201)|0);
    $203 = ($197|0)==(0);
    $204 = ($200|0)==(0);
    $205 = $203 & $204;
    $206 = $$3265 & 8;
    $207 = ($206|0)==(0);
    $or$cond283 = $207 | $205;
    $208 = $$1236 >> 4;
    $209 = (11849 + ($208)|0);
    $$289 = $or$cond283 ? 11849 : $209;
    $$290 = $or$cond283 ? 0 : 2;
    $$0228 = $202;$$1233 = $$290;$$1238 = $$289;$$2256 = $$1255;$$4266 = $$3265;$248 = $197;$250 = $200;
    label = 67;
   }
   else if ((label|0) == 66) {
    label = 0;
    $244 = (_fmt_u($242,$243,$11)|0);
    $$0228 = $244;$$1233 = $$0232;$$1238 = $$0237;$$2256 = $$0254;$$4266 = $$1263$;$248 = $242;$250 = $243;
    label = 67;
   }
   else if ((label|0) == 71) {
    label = 0;
    $272 = (_memchr($$1,0,$$0254)|0);
    $273 = ($272|0)==(0|0);
    $274 = $272;
    $275 = $$1;
    $276 = (($274) - ($275))|0;
    $277 = (($$1) + ($$0254)|0);
    $$3257 = $273 ? $$0254 : $276;
    $$1250 = $273 ? $277 : $272;
    $$2 = $$1;$$2234 = 0;$$2239 = 11849;$$2251 = $$1250;$$5 = $$3257;$$6268 = $164;
   }
   else if ((label|0) == 75) {
    label = 0;
    $$0229322 = $331;$$0240321 = 0;$$1244320 = 0;
    while(1) {
     $285 = HEAP32[$$0229322>>2]|0;
     $286 = ($285|0)==(0);
     if ($286) {
      $$0240$lcssa = $$0240321;$$2245 = $$1244320;
      break;
     }
     $287 = (_wctomb($9,$285)|0);
     $288 = ($287|0)<(0);
     $289 = (($$4258355) - ($$0240321))|0;
     $290 = ($287>>>0)>($289>>>0);
     $or$cond286 = $288 | $290;
     if ($or$cond286) {
      $$0240$lcssa = $$0240321;$$2245 = $287;
      break;
     }
     $291 = ((($$0229322)) + 4|0);
     $292 = (($287) + ($$0240321))|0;
     $293 = ($$4258355>>>0)>($292>>>0);
     if ($293) {
      $$0229322 = $291;$$0240321 = $292;$$1244320 = $287;
     } else {
      $$0240$lcssa = $292;$$2245 = $287;
      break;
     }
    }
    $294 = ($$2245|0)<(0);
    if ($294) {
     $$0 = -1;
     break L1;
    }
    _pad_674($0,32,$$1260,$$0240$lcssa,$$1263$);
    $295 = ($$0240$lcssa|0)==(0);
    if ($295) {
     $$0240$lcssa357 = 0;
     label = 84;
    } else {
     $$1230333 = $331;$$1241332 = 0;
     while(1) {
      $296 = HEAP32[$$1230333>>2]|0;
      $297 = ($296|0)==(0);
      if ($297) {
       $$0240$lcssa357 = $$0240$lcssa;
       label = 84;
       break L95;
      }
      $298 = (_wctomb($9,$296)|0);
      $299 = (($298) + ($$1241332))|0;
      $300 = ($299|0)>($$0240$lcssa|0);
      if ($300) {
       $$0240$lcssa357 = $$0240$lcssa;
       label = 84;
       break L95;
      }
      $301 = ((($$1230333)) + 4|0);
      _out($0,$9,$298);
      $302 = ($299>>>0)<($$0240$lcssa>>>0);
      if ($302) {
       $$1230333 = $301;$$1241332 = $299;
      } else {
       $$0240$lcssa357 = $$0240$lcssa;
       label = 84;
       break;
      }
     }
    }
   }
  } while(0);
  if ((label|0) == 67) {
   label = 0;
   $245 = ($$2256|0)>(-1);
   $246 = $$4266 & -65537;
   $$$4266 = $245 ? $246 : $$4266;
   $247 = ($248|0)!=(0);
   $249 = ($250|0)!=(0);
   $251 = $247 | $249;
   $252 = ($$2256|0)!=(0);
   $or$cond = $252 | $251;
   $253 = $$0228;
   $254 = (($12) - ($253))|0;
   $255 = $251 ^ 1;
   $256 = $255&1;
   $257 = (($256) + ($254))|0;
   $258 = ($$2256|0)>($257|0);
   $$2256$ = $258 ? $$2256 : $257;
   $$2256$$$2256 = $or$cond ? $$2256$ : $$2256;
   $$0228$ = $or$cond ? $$0228 : $11;
   $$2 = $$0228$;$$2234 = $$1233;$$2239 = $$1238;$$2251 = $11;$$5 = $$2256$$$2256;$$6268 = $$$4266;
  }
  else if ((label|0) == 84) {
   label = 0;
   $303 = $$1263$ ^ 8192;
   _pad_674($0,32,$$1260,$$0240$lcssa357,$303);
   $304 = ($$1260|0)>($$0240$lcssa357|0);
   $305 = $304 ? $$1260 : $$0240$lcssa357;
   $$0243 = $305;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
   continue;
  }
  $308 = $$2251;
  $309 = $$2;
  $310 = (($308) - ($309))|0;
  $311 = ($$5|0)<($310|0);
  $$$5 = $311 ? $310 : $$5;
  $312 = (($$$5) + ($$2234))|0;
  $313 = ($$1260|0)<($312|0);
  $$2261 = $313 ? $312 : $$1260;
  _pad_674($0,32,$$2261,$312,$$6268);
  _out($0,$$2239,$$2234);
  $314 = $$6268 ^ 65536;
  _pad_674($0,48,$$2261,$312,$314);
  _pad_674($0,48,$$$5,$310,0);
  _out($0,$$2,$310);
  $315 = $$6268 ^ 8192;
  _pad_674($0,32,$$2261,$312,$315);
  $$0243 = $$2261;$$0247 = $$1248;$$0269 = $$3272;$21 = $132;
 }
 L114: do {
  if ((label|0) == 87) {
   $316 = ($0|0)==(0|0);
   if ($316) {
    $317 = ($$0269|0)==(0);
    if ($317) {
     $$0 = 0;
    } else {
     $$2242305 = 1;
     while(1) {
      $318 = (($4) + ($$2242305<<2)|0);
      $319 = HEAP32[$318>>2]|0;
      $320 = ($319|0)==(0);
      if ($320) {
       $$3303 = $$2242305;
       break;
      }
      $321 = (($3) + ($$2242305<<3)|0);
      _pop_arg($321,$319,$2);
      $322 = (($$2242305) + 1)|0;
      $323 = ($322|0)<(10);
      if ($323) {
       $$2242305 = $322;
      } else {
       $$0 = 1;
       break L114;
      }
     }
     while(1) {
      $326 = (($4) + ($$3303<<2)|0);
      $327 = HEAP32[$326>>2]|0;
      $328 = ($327|0)==(0);
      $325 = (($$3303) + 1)|0;
      if (!($328)) {
       $$0 = -1;
       break L114;
      }
      $324 = ($325|0)<(10);
      if ($324) {
       $$3303 = $325;
      } else {
       $$0 = 1;
       break;
      }
     }
    }
   } else {
    $$0 = $$1248;
   }
  }
 } while(0);
 STACKTOP = sp;return ($$0|0);
}
function ___lockfile($0) {
 $0 = $0|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 return 0;
}
function ___unlockfile($0) {
 $0 = $0|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 return;
}
function _out($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $3 = 0, $4 = 0, $5 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = HEAP32[$0>>2]|0;
 $4 = $3 & 32;
 $5 = ($4|0)==(0);
 if ($5) {
  (___fwritex($1,$2,$0)|0);
 }
 return;
}
function _getint($0) {
 $0 = $0|0;
 var $$0$lcssa = 0, $$06 = 0, $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $isdigit = 0, $isdigit5 = 0, $isdigittmp = 0, $isdigittmp4 = 0, $isdigittmp7 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = HEAP32[$0>>2]|0;
 $2 = HEAP8[$1>>0]|0;
 $3 = $2 << 24 >> 24;
 $isdigittmp4 = (($3) + -48)|0;
 $isdigit5 = ($isdigittmp4>>>0)<(10);
 if ($isdigit5) {
  $$06 = 0;$7 = $1;$isdigittmp7 = $isdigittmp4;
  while(1) {
   $4 = ($$06*10)|0;
   $5 = (($isdigittmp7) + ($4))|0;
   $6 = ((($7)) + 1|0);
   HEAP32[$0>>2] = $6;
   $8 = HEAP8[$6>>0]|0;
   $9 = $8 << 24 >> 24;
   $isdigittmp = (($9) + -48)|0;
   $isdigit = ($isdigittmp>>>0)<(10);
   if ($isdigit) {
    $$06 = $5;$7 = $6;$isdigittmp7 = $isdigittmp;
   } else {
    $$0$lcssa = $5;
    break;
   }
  }
 } else {
  $$0$lcssa = 0;
 }
 return ($$0$lcssa|0);
}
function _pop_arg($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$mask = 0, $$mask31 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0.0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0;
 var $116 = 0.0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0;
 var $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0;
 var $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0;
 var $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0;
 var $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $arglist_current = 0, $arglist_current11 = 0, $arglist_current14 = 0, $arglist_current17 = 0;
 var $arglist_current2 = 0, $arglist_current20 = 0, $arglist_current23 = 0, $arglist_current26 = 0, $arglist_current5 = 0, $arglist_current8 = 0, $arglist_next = 0, $arglist_next12 = 0, $arglist_next15 = 0, $arglist_next18 = 0, $arglist_next21 = 0, $arglist_next24 = 0, $arglist_next27 = 0, $arglist_next3 = 0, $arglist_next6 = 0, $arglist_next9 = 0, $expanded = 0, $expanded28 = 0, $expanded30 = 0, $expanded31 = 0;
 var $expanded32 = 0, $expanded34 = 0, $expanded35 = 0, $expanded37 = 0, $expanded38 = 0, $expanded39 = 0, $expanded41 = 0, $expanded42 = 0, $expanded44 = 0, $expanded45 = 0, $expanded46 = 0, $expanded48 = 0, $expanded49 = 0, $expanded51 = 0, $expanded52 = 0, $expanded53 = 0, $expanded55 = 0, $expanded56 = 0, $expanded58 = 0, $expanded59 = 0;
 var $expanded60 = 0, $expanded62 = 0, $expanded63 = 0, $expanded65 = 0, $expanded66 = 0, $expanded67 = 0, $expanded69 = 0, $expanded70 = 0, $expanded72 = 0, $expanded73 = 0, $expanded74 = 0, $expanded76 = 0, $expanded77 = 0, $expanded79 = 0, $expanded80 = 0, $expanded81 = 0, $expanded83 = 0, $expanded84 = 0, $expanded86 = 0, $expanded87 = 0;
 var $expanded88 = 0, $expanded90 = 0, $expanded91 = 0, $expanded93 = 0, $expanded94 = 0, $expanded95 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($1>>>0)>(20);
 L1: do {
  if (!($3)) {
   do {
    switch ($1|0) {
    case 9:  {
     $arglist_current = HEAP32[$2>>2]|0;
     $4 = $arglist_current;
     $5 = ((0) + 4|0);
     $expanded28 = $5;
     $expanded = (($expanded28) - 1)|0;
     $6 = (($4) + ($expanded))|0;
     $7 = ((0) + 4|0);
     $expanded32 = $7;
     $expanded31 = (($expanded32) - 1)|0;
     $expanded30 = $expanded31 ^ -1;
     $8 = $6 & $expanded30;
     $9 = $8;
     $10 = HEAP32[$9>>2]|0;
     $arglist_next = ((($9)) + 4|0);
     HEAP32[$2>>2] = $arglist_next;
     HEAP32[$0>>2] = $10;
     break L1;
     break;
    }
    case 10:  {
     $arglist_current2 = HEAP32[$2>>2]|0;
     $11 = $arglist_current2;
     $12 = ((0) + 4|0);
     $expanded35 = $12;
     $expanded34 = (($expanded35) - 1)|0;
     $13 = (($11) + ($expanded34))|0;
     $14 = ((0) + 4|0);
     $expanded39 = $14;
     $expanded38 = (($expanded39) - 1)|0;
     $expanded37 = $expanded38 ^ -1;
     $15 = $13 & $expanded37;
     $16 = $15;
     $17 = HEAP32[$16>>2]|0;
     $arglist_next3 = ((($16)) + 4|0);
     HEAP32[$2>>2] = $arglist_next3;
     $18 = ($17|0)<(0);
     $19 = $18 << 31 >> 31;
     $20 = $0;
     $21 = $20;
     HEAP32[$21>>2] = $17;
     $22 = (($20) + 4)|0;
     $23 = $22;
     HEAP32[$23>>2] = $19;
     break L1;
     break;
    }
    case 11:  {
     $arglist_current5 = HEAP32[$2>>2]|0;
     $24 = $arglist_current5;
     $25 = ((0) + 4|0);
     $expanded42 = $25;
     $expanded41 = (($expanded42) - 1)|0;
     $26 = (($24) + ($expanded41))|0;
     $27 = ((0) + 4|0);
     $expanded46 = $27;
     $expanded45 = (($expanded46) - 1)|0;
     $expanded44 = $expanded45 ^ -1;
     $28 = $26 & $expanded44;
     $29 = $28;
     $30 = HEAP32[$29>>2]|0;
     $arglist_next6 = ((($29)) + 4|0);
     HEAP32[$2>>2] = $arglist_next6;
     $31 = $0;
     $32 = $31;
     HEAP32[$32>>2] = $30;
     $33 = (($31) + 4)|0;
     $34 = $33;
     HEAP32[$34>>2] = 0;
     break L1;
     break;
    }
    case 12:  {
     $arglist_current8 = HEAP32[$2>>2]|0;
     $35 = $arglist_current8;
     $36 = ((0) + 8|0);
     $expanded49 = $36;
     $expanded48 = (($expanded49) - 1)|0;
     $37 = (($35) + ($expanded48))|0;
     $38 = ((0) + 8|0);
     $expanded53 = $38;
     $expanded52 = (($expanded53) - 1)|0;
     $expanded51 = $expanded52 ^ -1;
     $39 = $37 & $expanded51;
     $40 = $39;
     $41 = $40;
     $42 = $41;
     $43 = HEAP32[$42>>2]|0;
     $44 = (($41) + 4)|0;
     $45 = $44;
     $46 = HEAP32[$45>>2]|0;
     $arglist_next9 = ((($40)) + 8|0);
     HEAP32[$2>>2] = $arglist_next9;
     $47 = $0;
     $48 = $47;
     HEAP32[$48>>2] = $43;
     $49 = (($47) + 4)|0;
     $50 = $49;
     HEAP32[$50>>2] = $46;
     break L1;
     break;
    }
    case 13:  {
     $arglist_current11 = HEAP32[$2>>2]|0;
     $51 = $arglist_current11;
     $52 = ((0) + 4|0);
     $expanded56 = $52;
     $expanded55 = (($expanded56) - 1)|0;
     $53 = (($51) + ($expanded55))|0;
     $54 = ((0) + 4|0);
     $expanded60 = $54;
     $expanded59 = (($expanded60) - 1)|0;
     $expanded58 = $expanded59 ^ -1;
     $55 = $53 & $expanded58;
     $56 = $55;
     $57 = HEAP32[$56>>2]|0;
     $arglist_next12 = ((($56)) + 4|0);
     HEAP32[$2>>2] = $arglist_next12;
     $58 = $57&65535;
     $59 = $58 << 16 >> 16;
     $60 = ($59|0)<(0);
     $61 = $60 << 31 >> 31;
     $62 = $0;
     $63 = $62;
     HEAP32[$63>>2] = $59;
     $64 = (($62) + 4)|0;
     $65 = $64;
     HEAP32[$65>>2] = $61;
     break L1;
     break;
    }
    case 14:  {
     $arglist_current14 = HEAP32[$2>>2]|0;
     $66 = $arglist_current14;
     $67 = ((0) + 4|0);
     $expanded63 = $67;
     $expanded62 = (($expanded63) - 1)|0;
     $68 = (($66) + ($expanded62))|0;
     $69 = ((0) + 4|0);
     $expanded67 = $69;
     $expanded66 = (($expanded67) - 1)|0;
     $expanded65 = $expanded66 ^ -1;
     $70 = $68 & $expanded65;
     $71 = $70;
     $72 = HEAP32[$71>>2]|0;
     $arglist_next15 = ((($71)) + 4|0);
     HEAP32[$2>>2] = $arglist_next15;
     $$mask31 = $72 & 65535;
     $73 = $0;
     $74 = $73;
     HEAP32[$74>>2] = $$mask31;
     $75 = (($73) + 4)|0;
     $76 = $75;
     HEAP32[$76>>2] = 0;
     break L1;
     break;
    }
    case 15:  {
     $arglist_current17 = HEAP32[$2>>2]|0;
     $77 = $arglist_current17;
     $78 = ((0) + 4|0);
     $expanded70 = $78;
     $expanded69 = (($expanded70) - 1)|0;
     $79 = (($77) + ($expanded69))|0;
     $80 = ((0) + 4|0);
     $expanded74 = $80;
     $expanded73 = (($expanded74) - 1)|0;
     $expanded72 = $expanded73 ^ -1;
     $81 = $79 & $expanded72;
     $82 = $81;
     $83 = HEAP32[$82>>2]|0;
     $arglist_next18 = ((($82)) + 4|0);
     HEAP32[$2>>2] = $arglist_next18;
     $84 = $83&255;
     $85 = $84 << 24 >> 24;
     $86 = ($85|0)<(0);
     $87 = $86 << 31 >> 31;
     $88 = $0;
     $89 = $88;
     HEAP32[$89>>2] = $85;
     $90 = (($88) + 4)|0;
     $91 = $90;
     HEAP32[$91>>2] = $87;
     break L1;
     break;
    }
    case 16:  {
     $arglist_current20 = HEAP32[$2>>2]|0;
     $92 = $arglist_current20;
     $93 = ((0) + 4|0);
     $expanded77 = $93;
     $expanded76 = (($expanded77) - 1)|0;
     $94 = (($92) + ($expanded76))|0;
     $95 = ((0) + 4|0);
     $expanded81 = $95;
     $expanded80 = (($expanded81) - 1)|0;
     $expanded79 = $expanded80 ^ -1;
     $96 = $94 & $expanded79;
     $97 = $96;
     $98 = HEAP32[$97>>2]|0;
     $arglist_next21 = ((($97)) + 4|0);
     HEAP32[$2>>2] = $arglist_next21;
     $$mask = $98 & 255;
     $99 = $0;
     $100 = $99;
     HEAP32[$100>>2] = $$mask;
     $101 = (($99) + 4)|0;
     $102 = $101;
     HEAP32[$102>>2] = 0;
     break L1;
     break;
    }
    case 17:  {
     $arglist_current23 = HEAP32[$2>>2]|0;
     $103 = $arglist_current23;
     $104 = ((0) + 8|0);
     $expanded84 = $104;
     $expanded83 = (($expanded84) - 1)|0;
     $105 = (($103) + ($expanded83))|0;
     $106 = ((0) + 8|0);
     $expanded88 = $106;
     $expanded87 = (($expanded88) - 1)|0;
     $expanded86 = $expanded87 ^ -1;
     $107 = $105 & $expanded86;
     $108 = $107;
     $109 = +HEAPF64[$108>>3];
     $arglist_next24 = ((($108)) + 8|0);
     HEAP32[$2>>2] = $arglist_next24;
     HEAPF64[$0>>3] = $109;
     break L1;
     break;
    }
    case 18:  {
     $arglist_current26 = HEAP32[$2>>2]|0;
     $110 = $arglist_current26;
     $111 = ((0) + 8|0);
     $expanded91 = $111;
     $expanded90 = (($expanded91) - 1)|0;
     $112 = (($110) + ($expanded90))|0;
     $113 = ((0) + 8|0);
     $expanded95 = $113;
     $expanded94 = (($expanded95) - 1)|0;
     $expanded93 = $expanded94 ^ -1;
     $114 = $112 & $expanded93;
     $115 = $114;
     $116 = +HEAPF64[$115>>3];
     $arglist_next27 = ((($115)) + 8|0);
     HEAP32[$2>>2] = $arglist_next27;
     HEAPF64[$0>>3] = $116;
     break L1;
     break;
    }
    default: {
     break L1;
    }
    }
   } while(0);
  }
 } while(0);
 return;
}
function _fmt_x($0,$1,$2,$3) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 var $$05$lcssa = 0, $$056 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0;
 var sp = 0;
 sp = STACKTOP;
 $4 = ($0|0)==(0);
 $5 = ($1|0)==(0);
 $6 = $4 & $5;
 if ($6) {
  $$05$lcssa = $2;
 } else {
  $$056 = $2;$15 = $1;$8 = $0;
  while(1) {
   $7 = $8 & 15;
   $9 = (11901 + ($7)|0);
   $10 = HEAP8[$9>>0]|0;
   $11 = $10&255;
   $12 = $11 | $3;
   $13 = $12&255;
   $14 = ((($$056)) + -1|0);
   HEAP8[$14>>0] = $13;
   $16 = (_bitshift64Lshr(($8|0),($15|0),4)|0);
   $17 = tempRet0;
   $18 = ($16|0)==(0);
   $19 = ($17|0)==(0);
   $20 = $18 & $19;
   if ($20) {
    $$05$lcssa = $14;
    break;
   } else {
    $$056 = $14;$15 = $17;$8 = $16;
   }
  }
 }
 return ($$05$lcssa|0);
}
function _fmt_o($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$0$lcssa = 0, $$06 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($0|0)==(0);
 $4 = ($1|0)==(0);
 $5 = $3 & $4;
 if ($5) {
  $$0$lcssa = $2;
 } else {
  $$06 = $2;$11 = $1;$7 = $0;
  while(1) {
   $6 = $7&255;
   $8 = $6 & 7;
   $9 = $8 | 48;
   $10 = ((($$06)) + -1|0);
   HEAP8[$10>>0] = $9;
   $12 = (_bitshift64Lshr(($7|0),($11|0),3)|0);
   $13 = tempRet0;
   $14 = ($12|0)==(0);
   $15 = ($13|0)==(0);
   $16 = $14 & $15;
   if ($16) {
    $$0$lcssa = $10;
    break;
   } else {
    $$06 = $10;$11 = $13;$7 = $12;
   }
  }
 }
 return ($$0$lcssa|0);
}
function _fmt_u($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$010$lcssa$off0 = 0, $$012 = 0, $$09$lcssa = 0, $$0914 = 0, $$1$lcssa = 0, $$111 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($1>>>0)>(0);
 $4 = ($0>>>0)>(4294967295);
 $5 = ($1|0)==(0);
 $6 = $5 & $4;
 $7 = $3 | $6;
 if ($7) {
  $$0914 = $2;$8 = $0;$9 = $1;
  while(1) {
   $10 = (___uremdi3(($8|0),($9|0),10,0)|0);
   $11 = tempRet0;
   $12 = $10&255;
   $13 = $12 | 48;
   $14 = ((($$0914)) + -1|0);
   HEAP8[$14>>0] = $13;
   $15 = (___udivdi3(($8|0),($9|0),10,0)|0);
   $16 = tempRet0;
   $17 = ($9>>>0)>(9);
   $18 = ($8>>>0)>(4294967295);
   $19 = ($9|0)==(9);
   $20 = $19 & $18;
   $21 = $17 | $20;
   if ($21) {
    $$0914 = $14;$8 = $15;$9 = $16;
   } else {
    break;
   }
  }
  $$010$lcssa$off0 = $15;$$09$lcssa = $14;
 } else {
  $$010$lcssa$off0 = $0;$$09$lcssa = $2;
 }
 $22 = ($$010$lcssa$off0|0)==(0);
 if ($22) {
  $$1$lcssa = $$09$lcssa;
 } else {
  $$012 = $$010$lcssa$off0;$$111 = $$09$lcssa;
  while(1) {
   $23 = (($$012>>>0) % 10)&-1;
   $24 = $23 | 48;
   $25 = $24&255;
   $26 = ((($$111)) + -1|0);
   HEAP8[$26>>0] = $25;
   $27 = (($$012>>>0) / 10)&-1;
   $28 = ($$012>>>0)<(10);
   if ($28) {
    $$1$lcssa = $26;
    break;
   } else {
    $$012 = $27;$$111 = $26;
   }
  }
 }
 return ($$1$lcssa|0);
}
function _strerror($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = (___pthread_self_105()|0);
 $2 = ((($1)) + 188|0);
 $3 = HEAP32[$2>>2]|0;
 $4 = (___strerror_l($0,$3)|0);
 return ($4|0);
}
function _memchr($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$0$lcssa = 0, $$035$lcssa = 0, $$035$lcssa65 = 0, $$03555 = 0, $$036$lcssa = 0, $$036$lcssa64 = 0, $$03654 = 0, $$046 = 0, $$137$lcssa = 0, $$13745 = 0, $$140 = 0, $$2 = 0, $$23839 = 0, $$3 = 0, $$lcssa = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0;
 var $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0;
 var $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond53 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = $1 & 255;
 $4 = $0;
 $5 = $4 & 3;
 $6 = ($5|0)!=(0);
 $7 = ($2|0)!=(0);
 $or$cond53 = $7 & $6;
 L1: do {
  if ($or$cond53) {
   $8 = $1&255;
   $$03555 = $0;$$03654 = $2;
   while(1) {
    $9 = HEAP8[$$03555>>0]|0;
    $10 = ($9<<24>>24)==($8<<24>>24);
    if ($10) {
     $$035$lcssa65 = $$03555;$$036$lcssa64 = $$03654;
     label = 6;
     break L1;
    }
    $11 = ((($$03555)) + 1|0);
    $12 = (($$03654) + -1)|0;
    $13 = $11;
    $14 = $13 & 3;
    $15 = ($14|0)!=(0);
    $16 = ($12|0)!=(0);
    $or$cond = $16 & $15;
    if ($or$cond) {
     $$03555 = $11;$$03654 = $12;
    } else {
     $$035$lcssa = $11;$$036$lcssa = $12;$$lcssa = $16;
     label = 5;
     break;
    }
   }
  } else {
   $$035$lcssa = $0;$$036$lcssa = $2;$$lcssa = $7;
   label = 5;
  }
 } while(0);
 if ((label|0) == 5) {
  if ($$lcssa) {
   $$035$lcssa65 = $$035$lcssa;$$036$lcssa64 = $$036$lcssa;
   label = 6;
  } else {
   $$2 = $$035$lcssa;$$3 = 0;
  }
 }
 L8: do {
  if ((label|0) == 6) {
   $17 = HEAP8[$$035$lcssa65>>0]|0;
   $18 = $1&255;
   $19 = ($17<<24>>24)==($18<<24>>24);
   if ($19) {
    $$2 = $$035$lcssa65;$$3 = $$036$lcssa64;
   } else {
    $20 = Math_imul($3, 16843009)|0;
    $21 = ($$036$lcssa64>>>0)>(3);
    L11: do {
     if ($21) {
      $$046 = $$035$lcssa65;$$13745 = $$036$lcssa64;
      while(1) {
       $22 = HEAP32[$$046>>2]|0;
       $23 = $22 ^ $20;
       $24 = (($23) + -16843009)|0;
       $25 = $23 & -2139062144;
       $26 = $25 ^ -2139062144;
       $27 = $26 & $24;
       $28 = ($27|0)==(0);
       if (!($28)) {
        break;
       }
       $29 = ((($$046)) + 4|0);
       $30 = (($$13745) + -4)|0;
       $31 = ($30>>>0)>(3);
       if ($31) {
        $$046 = $29;$$13745 = $30;
       } else {
        $$0$lcssa = $29;$$137$lcssa = $30;
        label = 11;
        break L11;
       }
      }
      $$140 = $$046;$$23839 = $$13745;
     } else {
      $$0$lcssa = $$035$lcssa65;$$137$lcssa = $$036$lcssa64;
      label = 11;
     }
    } while(0);
    if ((label|0) == 11) {
     $32 = ($$137$lcssa|0)==(0);
     if ($32) {
      $$2 = $$0$lcssa;$$3 = 0;
      break;
     } else {
      $$140 = $$0$lcssa;$$23839 = $$137$lcssa;
     }
    }
    while(1) {
     $33 = HEAP8[$$140>>0]|0;
     $34 = ($33<<24>>24)==($18<<24>>24);
     if ($34) {
      $$2 = $$140;$$3 = $$23839;
      break L8;
     }
     $35 = ((($$140)) + 1|0);
     $36 = (($$23839) + -1)|0;
     $37 = ($36|0)==(0);
     if ($37) {
      $$2 = $35;$$3 = 0;
      break;
     } else {
      $$140 = $35;$$23839 = $36;
     }
    }
   }
  }
 } while(0);
 $38 = ($$3|0)!=(0);
 $39 = $38 ? $$2 : 0;
 return ($39|0);
}
function _pad_674($0,$1,$2,$3,$4) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 var $$0$lcssa = 0, $$011 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 256|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(256|0);
 $5 = sp;
 $6 = $4 & 73728;
 $7 = ($6|0)==(0);
 $8 = ($2|0)>($3|0);
 $or$cond = $8 & $7;
 if ($or$cond) {
  $9 = (($2) - ($3))|0;
  $10 = ($9>>>0)<(256);
  $11 = $10 ? $9 : 256;
  _memset(($5|0),($1|0),($11|0))|0;
  $12 = ($9>>>0)>(255);
  if ($12) {
   $13 = (($2) - ($3))|0;
   $$011 = $9;
   while(1) {
    _out($0,$5,256);
    $14 = (($$011) + -256)|0;
    $15 = ($14>>>0)>(255);
    if ($15) {
     $$011 = $14;
    } else {
     break;
    }
   }
   $16 = $13 & 255;
   $$0$lcssa = $16;
  } else {
   $$0$lcssa = $9;
  }
  _out($0,$5,$$0$lcssa);
 }
 STACKTOP = sp;return;
}
function _wctomb($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $2 = 0, $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ($0|0)==(0|0);
 if ($2) {
  $$0 = 0;
 } else {
  $3 = (_wcrtomb($0,$1,0)|0);
  $$0 = $3;
 }
 return ($$0|0);
}
function _fmt_fp($0,$1,$2,$3,$4,$5) {
 $0 = $0|0;
 $1 = +$1;
 $2 = $2|0;
 $3 = $3|0;
 $4 = $4|0;
 $5 = $5|0;
 var $$ = 0, $$$ = 0, $$$$559 = 0.0, $$$3484 = 0, $$$3484691 = 0, $$$3484692 = 0, $$$3501 = 0, $$$4502 = 0, $$$542 = 0.0, $$$559 = 0.0, $$0 = 0, $$0463$lcssa = 0, $$0463584 = 0, $$0464594 = 0, $$0471 = 0.0, $$0479 = 0, $$0487642 = 0, $$0488 = 0, $$0488653 = 0, $$0488655 = 0;
 var $$0496$$9 = 0, $$0497654 = 0, $$0498 = 0, $$0509582 = 0.0, $$0510 = 0, $$0511 = 0, $$0514637 = 0, $$0520 = 0, $$0521 = 0, $$0521$ = 0, $$0523 = 0, $$0525 = 0, $$0527 = 0, $$0527629 = 0, $$0527631 = 0, $$0530636 = 0, $$1465 = 0, $$1467 = 0.0, $$1469 = 0.0, $$1472 = 0.0;
 var $$1480 = 0, $$1482$lcssa = 0, $$1482661 = 0, $$1489641 = 0, $$1499$lcssa = 0, $$1499660 = 0, $$1508583 = 0, $$1512$lcssa = 0, $$1512607 = 0, $$1515 = 0, $$1524 = 0, $$1526 = 0, $$1528614 = 0, $$1531$lcssa = 0, $$1531630 = 0, $$1598 = 0, $$2 = 0, $$2473 = 0.0, $$2476 = 0, $$2476$$547 = 0;
 var $$2476$$549 = 0, $$2483$ph = 0, $$2500 = 0, $$2513 = 0, $$2516618 = 0, $$2529 = 0, $$2532617 = 0, $$3 = 0.0, $$3477 = 0, $$3484$lcssa = 0, $$3484648 = 0, $$3501$lcssa = 0, $$3501647 = 0, $$3533613 = 0, $$4 = 0.0, $$4478$lcssa = 0, $$4478590 = 0, $$4492 = 0, $$4502 = 0, $$4518 = 0;
 var $$5$lcssa = 0, $$534$ = 0, $$539 = 0, $$539$ = 0, $$542 = 0.0, $$546 = 0, $$548 = 0, $$5486$lcssa = 0, $$5486623 = 0, $$5493597 = 0, $$5519$ph = 0, $$555 = 0, $$556 = 0, $$559 = 0.0, $$5602 = 0, $$6 = 0, $$6494589 = 0, $$7495601 = 0, $$7505 = 0, $$7505$ = 0;
 var $$7505$ph = 0, $$8 = 0, $$9$ph = 0, $$lcssa673 = 0, $$neg = 0, $$neg567 = 0, $$pn = 0, $$pn566 = 0, $$pr = 0, $$pr564 = 0, $$pre = 0, $$pre$phi690Z2D = 0, $$pre689 = 0, $$sink545$lcssa = 0, $$sink545622 = 0, $$sink562 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0;
 var $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0.0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0.0, $117 = 0.0, $118 = 0.0, $119 = 0, $12 = 0, $120 = 0;
 var $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0;
 var $14 = 0.0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0, $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0;
 var $158 = 0, $159 = 0, $16 = 0, $160 = 0, $161 = 0, $162 = 0, $163 = 0, $164 = 0, $165 = 0, $166 = 0, $167 = 0, $168 = 0, $169 = 0, $17 = 0, $170 = 0, $171 = 0, $172 = 0, $173 = 0, $174 = 0, $175 = 0;
 var $176 = 0, $177 = 0, $178 = 0, $179 = 0, $18 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0, $184 = 0, $185 = 0, $186 = 0, $187 = 0, $188 = 0, $189 = 0, $19 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0;
 var $194 = 0, $195 = 0, $196 = 0, $197 = 0, $198 = 0, $199 = 0, $20 = 0, $200 = 0, $201 = 0, $202 = 0, $203 = 0, $204 = 0, $205 = 0, $206 = 0, $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0;
 var $212 = 0, $213 = 0, $214 = 0, $215 = 0, $216 = 0, $217 = 0, $218 = 0, $219 = 0, $22 = 0, $220 = 0, $221 = 0, $222 = 0, $223 = 0, $224 = 0, $225 = 0, $226 = 0, $227 = 0, $228 = 0.0, $229 = 0.0, $23 = 0;
 var $230 = 0, $231 = 0.0, $232 = 0, $233 = 0, $234 = 0, $235 = 0, $236 = 0, $237 = 0, $238 = 0, $239 = 0, $24 = 0, $240 = 0, $241 = 0, $242 = 0, $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0;
 var $249 = 0, $25 = 0, $250 = 0, $251 = 0, $252 = 0, $253 = 0, $254 = 0, $255 = 0, $256 = 0, $257 = 0, $258 = 0, $259 = 0, $26 = 0, $260 = 0, $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0;
 var $267 = 0, $268 = 0, $269 = 0, $27 = 0, $270 = 0, $271 = 0, $272 = 0, $273 = 0, $274 = 0, $275 = 0, $276 = 0, $277 = 0, $278 = 0, $279 = 0, $28 = 0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0;
 var $285 = 0, $286 = 0, $287 = 0, $288 = 0, $289 = 0, $29 = 0, $290 = 0, $291 = 0, $292 = 0, $293 = 0, $294 = 0, $295 = 0, $296 = 0, $297 = 0, $298 = 0, $299 = 0, $30 = 0, $300 = 0, $301 = 0, $302 = 0;
 var $303 = 0, $304 = 0, $305 = 0, $306 = 0, $307 = 0, $308 = 0, $309 = 0, $31 = 0, $310 = 0, $311 = 0, $312 = 0, $313 = 0, $314 = 0, $315 = 0, $316 = 0, $317 = 0, $318 = 0, $319 = 0, $32 = 0, $320 = 0;
 var $321 = 0, $322 = 0, $323 = 0, $324 = 0, $325 = 0, $326 = 0, $327 = 0, $328 = 0, $329 = 0, $33 = 0, $330 = 0, $331 = 0, $332 = 0, $333 = 0, $334 = 0, $335 = 0, $336 = 0, $337 = 0, $338 = 0, $339 = 0;
 var $34 = 0, $340 = 0, $341 = 0, $342 = 0, $343 = 0, $344 = 0, $345 = 0, $346 = 0, $347 = 0, $348 = 0, $349 = 0, $35 = 0.0, $350 = 0, $351 = 0, $352 = 0, $353 = 0, $354 = 0, $355 = 0, $356 = 0, $357 = 0;
 var $358 = 0, $359 = 0, $36 = 0.0, $360 = 0, $361 = 0, $362 = 0, $363 = 0, $364 = 0, $365 = 0, $366 = 0, $367 = 0, $368 = 0, $369 = 0, $37 = 0, $370 = 0, $371 = 0, $372 = 0, $373 = 0, $374 = 0, $375 = 0;
 var $376 = 0, $377 = 0, $378 = 0, $379 = 0, $38 = 0, $380 = 0, $381 = 0, $382 = 0, $383 = 0, $384 = 0, $385 = 0, $386 = 0, $387 = 0, $388 = 0, $39 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0;
 var $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $50 = 0, $51 = 0.0, $52 = 0, $53 = 0, $54 = 0, $55 = 0.0, $56 = 0.0, $57 = 0.0, $58 = 0.0, $59 = 0.0, $6 = 0, $60 = 0.0, $61 = 0, $62 = 0, $63 = 0;
 var $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0;
 var $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0.0, $88 = 0.0, $89 = 0.0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $exitcond = 0;
 var $narrow = 0, $not$ = 0, $notlhs = 0, $notrhs = 0, $or$cond = 0, $or$cond3$not = 0, $or$cond537 = 0, $or$cond541 = 0, $or$cond544 = 0, $or$cond554 = 0, $or$cond6 = 0, $scevgep684 = 0, $scevgep684685 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 560|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(560|0);
 $6 = sp + 8|0;
 $7 = sp;
 $8 = sp + 524|0;
 $9 = $8;
 $10 = sp + 512|0;
 HEAP32[$7>>2] = 0;
 $11 = ((($10)) + 12|0);
 (___DOUBLE_BITS_675($1)|0);
 $12 = tempRet0;
 $13 = ($12|0)<(0);
 if ($13) {
  $14 = -$1;
  $$0471 = $14;$$0520 = 1;$$0521 = 11866;
 } else {
  $15 = $4 & 2048;
  $16 = ($15|0)==(0);
  $17 = $4 & 1;
  $18 = ($17|0)==(0);
  $$ = $18 ? (11867) : (11872);
  $$$ = $16 ? $$ : (11869);
  $19 = $4 & 2049;
  $narrow = ($19|0)!=(0);
  $$534$ = $narrow&1;
  $$0471 = $1;$$0520 = $$534$;$$0521 = $$$;
 }
 (___DOUBLE_BITS_675($$0471)|0);
 $20 = tempRet0;
 $21 = $20 & 2146435072;
 $22 = ($21>>>0)<(2146435072);
 $23 = (0)<(0);
 $24 = ($21|0)==(2146435072);
 $25 = $24 & $23;
 $26 = $22 | $25;
 do {
  if ($26) {
   $35 = (+_frexpl($$0471,$7));
   $36 = $35 * 2.0;
   $37 = $36 != 0.0;
   if ($37) {
    $38 = HEAP32[$7>>2]|0;
    $39 = (($38) + -1)|0;
    HEAP32[$7>>2] = $39;
   }
   $40 = $5 | 32;
   $41 = ($40|0)==(97);
   if ($41) {
    $42 = $5 & 32;
    $43 = ($42|0)==(0);
    $44 = ((($$0521)) + 9|0);
    $$0521$ = $43 ? $$0521 : $44;
    $45 = $$0520 | 2;
    $46 = ($3>>>0)>(11);
    $47 = (12 - ($3))|0;
    $48 = ($47|0)==(0);
    $49 = $46 | $48;
    do {
     if ($49) {
      $$1472 = $36;
     } else {
      $$0509582 = 8.0;$$1508583 = $47;
      while(1) {
       $50 = (($$1508583) + -1)|0;
       $51 = $$0509582 * 16.0;
       $52 = ($50|0)==(0);
       if ($52) {
        break;
       } else {
        $$0509582 = $51;$$1508583 = $50;
       }
      }
      $53 = HEAP8[$$0521$>>0]|0;
      $54 = ($53<<24>>24)==(45);
      if ($54) {
       $55 = -$36;
       $56 = $55 - $51;
       $57 = $51 + $56;
       $58 = -$57;
       $$1472 = $58;
       break;
      } else {
       $59 = $36 + $51;
       $60 = $59 - $51;
       $$1472 = $60;
       break;
      }
     }
    } while(0);
    $61 = HEAP32[$7>>2]|0;
    $62 = ($61|0)<(0);
    $63 = (0 - ($61))|0;
    $64 = $62 ? $63 : $61;
    $65 = ($64|0)<(0);
    $66 = $65 << 31 >> 31;
    $67 = (_fmt_u($64,$66,$11)|0);
    $68 = ($67|0)==($11|0);
    if ($68) {
     $69 = ((($10)) + 11|0);
     HEAP8[$69>>0] = 48;
     $$0511 = $69;
    } else {
     $$0511 = $67;
    }
    $70 = $61 >> 31;
    $71 = $70 & 2;
    $72 = (($71) + 43)|0;
    $73 = $72&255;
    $74 = ((($$0511)) + -1|0);
    HEAP8[$74>>0] = $73;
    $75 = (($5) + 15)|0;
    $76 = $75&255;
    $77 = ((($$0511)) + -2|0);
    HEAP8[$77>>0] = $76;
    $notrhs = ($3|0)<(1);
    $78 = $4 & 8;
    $79 = ($78|0)==(0);
    $$0523 = $8;$$2473 = $$1472;
    while(1) {
     $80 = (~~(($$2473)));
     $81 = (11901 + ($80)|0);
     $82 = HEAP8[$81>>0]|0;
     $83 = $82&255;
     $84 = $83 | $42;
     $85 = $84&255;
     $86 = ((($$0523)) + 1|0);
     HEAP8[$$0523>>0] = $85;
     $87 = (+($80|0));
     $88 = $$2473 - $87;
     $89 = $88 * 16.0;
     $90 = $86;
     $91 = (($90) - ($9))|0;
     $92 = ($91|0)==(1);
     if ($92) {
      $notlhs = $89 == 0.0;
      $or$cond3$not = $notrhs & $notlhs;
      $or$cond = $79 & $or$cond3$not;
      if ($or$cond) {
       $$1524 = $86;
      } else {
       $93 = ((($$0523)) + 2|0);
       HEAP8[$86>>0] = 46;
       $$1524 = $93;
      }
     } else {
      $$1524 = $86;
     }
     $94 = $89 != 0.0;
     if ($94) {
      $$0523 = $$1524;$$2473 = $89;
     } else {
      break;
     }
    }
    $95 = ($3|0)!=(0);
    $96 = $77;
    $97 = $11;
    $98 = $$1524;
    $99 = (($98) - ($9))|0;
    $100 = (($97) - ($96))|0;
    $101 = (($99) + -2)|0;
    $102 = ($101|0)<($3|0);
    $or$cond537 = $95 & $102;
    $103 = (($3) + 2)|0;
    $$pn = $or$cond537 ? $103 : $99;
    $$0525 = (($100) + ($45))|0;
    $104 = (($$0525) + ($$pn))|0;
    _pad_674($0,32,$2,$104,$4);
    _out($0,$$0521$,$45);
    $105 = $4 ^ 65536;
    _pad_674($0,48,$2,$104,$105);
    _out($0,$8,$99);
    $106 = (($$pn) - ($99))|0;
    _pad_674($0,48,$106,0,0);
    _out($0,$77,$100);
    $107 = $4 ^ 8192;
    _pad_674($0,32,$2,$104,$107);
    $$sink562 = $104;
    break;
   }
   $108 = ($3|0)<(0);
   $$539 = $108 ? 6 : $3;
   if ($37) {
    $109 = $36 * 268435456.0;
    $110 = HEAP32[$7>>2]|0;
    $111 = (($110) + -28)|0;
    HEAP32[$7>>2] = $111;
    $$3 = $109;$$pr = $111;
   } else {
    $$pre = HEAP32[$7>>2]|0;
    $$3 = $36;$$pr = $$pre;
   }
   $112 = ($$pr|0)<(0);
   $113 = ((($6)) + 288|0);
   $$556 = $112 ? $6 : $113;
   $$0498 = $$556;$$4 = $$3;
   while(1) {
    $114 = (~~(($$4))>>>0);
    HEAP32[$$0498>>2] = $114;
    $115 = ((($$0498)) + 4|0);
    $116 = (+($114>>>0));
    $117 = $$4 - $116;
    $118 = $117 * 1.0E+9;
    $119 = $118 != 0.0;
    if ($119) {
     $$0498 = $115;$$4 = $118;
    } else {
     break;
    }
   }
   $120 = ($$pr|0)>(0);
   if ($120) {
    $$1482661 = $$556;$$1499660 = $115;$122 = $$pr;
    while(1) {
     $121 = ($122|0)<(29);
     $123 = $121 ? $122 : 29;
     $$0488653 = ((($$1499660)) + -4|0);
     $124 = ($$0488653>>>0)<($$1482661>>>0);
     if ($124) {
      $$2483$ph = $$1482661;
     } else {
      $$0488655 = $$0488653;$$0497654 = 0;
      while(1) {
       $125 = HEAP32[$$0488655>>2]|0;
       $126 = (_bitshift64Shl(($125|0),0,($123|0))|0);
       $127 = tempRet0;
       $128 = (_i64Add(($126|0),($127|0),($$0497654|0),0)|0);
       $129 = tempRet0;
       $130 = (___uremdi3(($128|0),($129|0),1000000000,0)|0);
       $131 = tempRet0;
       HEAP32[$$0488655>>2] = $130;
       $132 = (___udivdi3(($128|0),($129|0),1000000000,0)|0);
       $133 = tempRet0;
       $$0488 = ((($$0488655)) + -4|0);
       $134 = ($$0488>>>0)<($$1482661>>>0);
       if ($134) {
        break;
       } else {
        $$0488655 = $$0488;$$0497654 = $132;
       }
      }
      $135 = ($132|0)==(0);
      if ($135) {
       $$2483$ph = $$1482661;
      } else {
       $136 = ((($$1482661)) + -4|0);
       HEAP32[$136>>2] = $132;
       $$2483$ph = $136;
      }
     }
     $$2500 = $$1499660;
     while(1) {
      $137 = ($$2500>>>0)>($$2483$ph>>>0);
      if (!($137)) {
       break;
      }
      $138 = ((($$2500)) + -4|0);
      $139 = HEAP32[$138>>2]|0;
      $140 = ($139|0)==(0);
      if ($140) {
       $$2500 = $138;
      } else {
       break;
      }
     }
     $141 = HEAP32[$7>>2]|0;
     $142 = (($141) - ($123))|0;
     HEAP32[$7>>2] = $142;
     $143 = ($142|0)>(0);
     if ($143) {
      $$1482661 = $$2483$ph;$$1499660 = $$2500;$122 = $142;
     } else {
      $$1482$lcssa = $$2483$ph;$$1499$lcssa = $$2500;$$pr564 = $142;
      break;
     }
    }
   } else {
    $$1482$lcssa = $$556;$$1499$lcssa = $115;$$pr564 = $$pr;
   }
   $144 = ($$pr564|0)<(0);
   if ($144) {
    $145 = (($$539) + 25)|0;
    $146 = (($145|0) / 9)&-1;
    $147 = (($146) + 1)|0;
    $148 = ($40|0)==(102);
    $$3484648 = $$1482$lcssa;$$3501647 = $$1499$lcssa;$150 = $$pr564;
    while(1) {
     $149 = (0 - ($150))|0;
     $151 = ($149|0)<(9);
     $152 = $151 ? $149 : 9;
     $153 = ($$3484648>>>0)<($$3501647>>>0);
     if ($153) {
      $157 = 1 << $152;
      $158 = (($157) + -1)|0;
      $159 = 1000000000 >>> $152;
      $$0487642 = 0;$$1489641 = $$3484648;
      while(1) {
       $160 = HEAP32[$$1489641>>2]|0;
       $161 = $160 & $158;
       $162 = $160 >>> $152;
       $163 = (($162) + ($$0487642))|0;
       HEAP32[$$1489641>>2] = $163;
       $164 = Math_imul($161, $159)|0;
       $165 = ((($$1489641)) + 4|0);
       $166 = ($165>>>0)<($$3501647>>>0);
       if ($166) {
        $$0487642 = $164;$$1489641 = $165;
       } else {
        break;
       }
      }
      $167 = HEAP32[$$3484648>>2]|0;
      $168 = ($167|0)==(0);
      $169 = ((($$3484648)) + 4|0);
      $$$3484 = $168 ? $169 : $$3484648;
      $170 = ($164|0)==(0);
      if ($170) {
       $$$3484692 = $$$3484;$$4502 = $$3501647;
      } else {
       $171 = ((($$3501647)) + 4|0);
       HEAP32[$$3501647>>2] = $164;
       $$$3484692 = $$$3484;$$4502 = $171;
      }
     } else {
      $154 = HEAP32[$$3484648>>2]|0;
      $155 = ($154|0)==(0);
      $156 = ((($$3484648)) + 4|0);
      $$$3484691 = $155 ? $156 : $$3484648;
      $$$3484692 = $$$3484691;$$4502 = $$3501647;
     }
     $172 = $148 ? $$556 : $$$3484692;
     $173 = $$4502;
     $174 = $172;
     $175 = (($173) - ($174))|0;
     $176 = $175 >> 2;
     $177 = ($176|0)>($147|0);
     $178 = (($172) + ($147<<2)|0);
     $$$4502 = $177 ? $178 : $$4502;
     $179 = HEAP32[$7>>2]|0;
     $180 = (($179) + ($152))|0;
     HEAP32[$7>>2] = $180;
     $181 = ($180|0)<(0);
     if ($181) {
      $$3484648 = $$$3484692;$$3501647 = $$$4502;$150 = $180;
     } else {
      $$3484$lcssa = $$$3484692;$$3501$lcssa = $$$4502;
      break;
     }
    }
   } else {
    $$3484$lcssa = $$1482$lcssa;$$3501$lcssa = $$1499$lcssa;
   }
   $182 = ($$3484$lcssa>>>0)<($$3501$lcssa>>>0);
   $183 = $$556;
   if ($182) {
    $184 = $$3484$lcssa;
    $185 = (($183) - ($184))|0;
    $186 = $185 >> 2;
    $187 = ($186*9)|0;
    $188 = HEAP32[$$3484$lcssa>>2]|0;
    $189 = ($188>>>0)<(10);
    if ($189) {
     $$1515 = $187;
    } else {
     $$0514637 = $187;$$0530636 = 10;
     while(1) {
      $190 = ($$0530636*10)|0;
      $191 = (($$0514637) + 1)|0;
      $192 = ($188>>>0)<($190>>>0);
      if ($192) {
       $$1515 = $191;
       break;
      } else {
       $$0514637 = $191;$$0530636 = $190;
      }
     }
    }
   } else {
    $$1515 = 0;
   }
   $193 = ($40|0)!=(102);
   $194 = $193 ? $$1515 : 0;
   $195 = (($$539) - ($194))|0;
   $196 = ($40|0)==(103);
   $197 = ($$539|0)!=(0);
   $198 = $197 & $196;
   $$neg = $198 << 31 >> 31;
   $199 = (($195) + ($$neg))|0;
   $200 = $$3501$lcssa;
   $201 = (($200) - ($183))|0;
   $202 = $201 >> 2;
   $203 = ($202*9)|0;
   $204 = (($203) + -9)|0;
   $205 = ($199|0)<($204|0);
   if ($205) {
    $206 = ((($$556)) + 4|0);
    $207 = (($199) + 9216)|0;
    $208 = (($207|0) / 9)&-1;
    $209 = (($208) + -1024)|0;
    $210 = (($206) + ($209<<2)|0);
    $211 = (($207|0) % 9)&-1;
    $$0527629 = (($211) + 1)|0;
    $212 = ($$0527629|0)<(9);
    if ($212) {
     $$0527631 = $$0527629;$$1531630 = 10;
     while(1) {
      $213 = ($$1531630*10)|0;
      $$0527 = (($$0527631) + 1)|0;
      $exitcond = ($$0527|0)==(9);
      if ($exitcond) {
       $$1531$lcssa = $213;
       break;
      } else {
       $$0527631 = $$0527;$$1531630 = $213;
      }
     }
    } else {
     $$1531$lcssa = 10;
    }
    $214 = HEAP32[$210>>2]|0;
    $215 = (($214>>>0) % ($$1531$lcssa>>>0))&-1;
    $216 = ($215|0)==(0);
    $217 = ((($210)) + 4|0);
    $218 = ($217|0)==($$3501$lcssa|0);
    $or$cond541 = $218 & $216;
    if ($or$cond541) {
     $$4492 = $210;$$4518 = $$1515;$$8 = $$3484$lcssa;
    } else {
     $219 = (($214>>>0) / ($$1531$lcssa>>>0))&-1;
     $220 = $219 & 1;
     $221 = ($220|0)==(0);
     $$542 = $221 ? 9007199254740992.0 : 9007199254740994.0;
     $222 = (($$1531$lcssa|0) / 2)&-1;
     $223 = ($215>>>0)<($222>>>0);
     $224 = ($215|0)==($222|0);
     $or$cond544 = $218 & $224;
     $$559 = $or$cond544 ? 1.0 : 1.5;
     $$$559 = $223 ? 0.5 : $$559;
     $225 = ($$0520|0)==(0);
     if ($225) {
      $$1467 = $$$559;$$1469 = $$542;
     } else {
      $226 = HEAP8[$$0521>>0]|0;
      $227 = ($226<<24>>24)==(45);
      $228 = -$$542;
      $229 = -$$$559;
      $$$542 = $227 ? $228 : $$542;
      $$$$559 = $227 ? $229 : $$$559;
      $$1467 = $$$$559;$$1469 = $$$542;
     }
     $230 = (($214) - ($215))|0;
     HEAP32[$210>>2] = $230;
     $231 = $$1469 + $$1467;
     $232 = $231 != $$1469;
     if ($232) {
      $233 = (($230) + ($$1531$lcssa))|0;
      HEAP32[$210>>2] = $233;
      $234 = ($233>>>0)>(999999999);
      if ($234) {
       $$5486623 = $$3484$lcssa;$$sink545622 = $210;
       while(1) {
        $235 = ((($$sink545622)) + -4|0);
        HEAP32[$$sink545622>>2] = 0;
        $236 = ($235>>>0)<($$5486623>>>0);
        if ($236) {
         $237 = ((($$5486623)) + -4|0);
         HEAP32[$237>>2] = 0;
         $$6 = $237;
        } else {
         $$6 = $$5486623;
        }
        $238 = HEAP32[$235>>2]|0;
        $239 = (($238) + 1)|0;
        HEAP32[$235>>2] = $239;
        $240 = ($239>>>0)>(999999999);
        if ($240) {
         $$5486623 = $$6;$$sink545622 = $235;
        } else {
         $$5486$lcssa = $$6;$$sink545$lcssa = $235;
         break;
        }
       }
      } else {
       $$5486$lcssa = $$3484$lcssa;$$sink545$lcssa = $210;
      }
      $241 = $$5486$lcssa;
      $242 = (($183) - ($241))|0;
      $243 = $242 >> 2;
      $244 = ($243*9)|0;
      $245 = HEAP32[$$5486$lcssa>>2]|0;
      $246 = ($245>>>0)<(10);
      if ($246) {
       $$4492 = $$sink545$lcssa;$$4518 = $244;$$8 = $$5486$lcssa;
      } else {
       $$2516618 = $244;$$2532617 = 10;
       while(1) {
        $247 = ($$2532617*10)|0;
        $248 = (($$2516618) + 1)|0;
        $249 = ($245>>>0)<($247>>>0);
        if ($249) {
         $$4492 = $$sink545$lcssa;$$4518 = $248;$$8 = $$5486$lcssa;
         break;
        } else {
         $$2516618 = $248;$$2532617 = $247;
        }
       }
      }
     } else {
      $$4492 = $210;$$4518 = $$1515;$$8 = $$3484$lcssa;
     }
    }
    $250 = ((($$4492)) + 4|0);
    $251 = ($$3501$lcssa>>>0)>($250>>>0);
    $$$3501 = $251 ? $250 : $$3501$lcssa;
    $$5519$ph = $$4518;$$7505$ph = $$$3501;$$9$ph = $$8;
   } else {
    $$5519$ph = $$1515;$$7505$ph = $$3501$lcssa;$$9$ph = $$3484$lcssa;
   }
   $$7505 = $$7505$ph;
   while(1) {
    $252 = ($$7505>>>0)>($$9$ph>>>0);
    if (!($252)) {
     $$lcssa673 = 0;
     break;
    }
    $253 = ((($$7505)) + -4|0);
    $254 = HEAP32[$253>>2]|0;
    $255 = ($254|0)==(0);
    if ($255) {
     $$7505 = $253;
    } else {
     $$lcssa673 = 1;
     break;
    }
   }
   $256 = (0 - ($$5519$ph))|0;
   do {
    if ($196) {
     $not$ = $197 ^ 1;
     $257 = $not$&1;
     $$539$ = (($257) + ($$539))|0;
     $258 = ($$539$|0)>($$5519$ph|0);
     $259 = ($$5519$ph|0)>(-5);
     $or$cond6 = $258 & $259;
     if ($or$cond6) {
      $260 = (($5) + -1)|0;
      $$neg567 = (($$539$) + -1)|0;
      $261 = (($$neg567) - ($$5519$ph))|0;
      $$0479 = $260;$$2476 = $261;
     } else {
      $262 = (($5) + -2)|0;
      $263 = (($$539$) + -1)|0;
      $$0479 = $262;$$2476 = $263;
     }
     $264 = $4 & 8;
     $265 = ($264|0)==(0);
     if ($265) {
      if ($$lcssa673) {
       $266 = ((($$7505)) + -4|0);
       $267 = HEAP32[$266>>2]|0;
       $268 = ($267|0)==(0);
       if ($268) {
        $$2529 = 9;
       } else {
        $269 = (($267>>>0) % 10)&-1;
        $270 = ($269|0)==(0);
        if ($270) {
         $$1528614 = 0;$$3533613 = 10;
         while(1) {
          $271 = ($$3533613*10)|0;
          $272 = (($$1528614) + 1)|0;
          $273 = (($267>>>0) % ($271>>>0))&-1;
          $274 = ($273|0)==(0);
          if ($274) {
           $$1528614 = $272;$$3533613 = $271;
          } else {
           $$2529 = $272;
           break;
          }
         }
        } else {
         $$2529 = 0;
        }
       }
      } else {
       $$2529 = 9;
      }
      $275 = $$0479 | 32;
      $276 = ($275|0)==(102);
      $277 = $$7505;
      $278 = (($277) - ($183))|0;
      $279 = $278 >> 2;
      $280 = ($279*9)|0;
      $281 = (($280) + -9)|0;
      if ($276) {
       $282 = (($281) - ($$2529))|0;
       $283 = ($282|0)>(0);
       $$546 = $283 ? $282 : 0;
       $284 = ($$2476|0)<($$546|0);
       $$2476$$547 = $284 ? $$2476 : $$546;
       $$1480 = $$0479;$$3477 = $$2476$$547;$$pre$phi690Z2D = 0;
       break;
      } else {
       $285 = (($281) + ($$5519$ph))|0;
       $286 = (($285) - ($$2529))|0;
       $287 = ($286|0)>(0);
       $$548 = $287 ? $286 : 0;
       $288 = ($$2476|0)<($$548|0);
       $$2476$$549 = $288 ? $$2476 : $$548;
       $$1480 = $$0479;$$3477 = $$2476$$549;$$pre$phi690Z2D = 0;
       break;
      }
     } else {
      $$1480 = $$0479;$$3477 = $$2476;$$pre$phi690Z2D = $264;
     }
    } else {
     $$pre689 = $4 & 8;
     $$1480 = $5;$$3477 = $$539;$$pre$phi690Z2D = $$pre689;
    }
   } while(0);
   $289 = $$3477 | $$pre$phi690Z2D;
   $290 = ($289|0)!=(0);
   $291 = $290&1;
   $292 = $$1480 | 32;
   $293 = ($292|0)==(102);
   if ($293) {
    $294 = ($$5519$ph|0)>(0);
    $295 = $294 ? $$5519$ph : 0;
    $$2513 = 0;$$pn566 = $295;
   } else {
    $296 = ($$5519$ph|0)<(0);
    $297 = $296 ? $256 : $$5519$ph;
    $298 = ($297|0)<(0);
    $299 = $298 << 31 >> 31;
    $300 = (_fmt_u($297,$299,$11)|0);
    $301 = $11;
    $302 = $300;
    $303 = (($301) - ($302))|0;
    $304 = ($303|0)<(2);
    if ($304) {
     $$1512607 = $300;
     while(1) {
      $305 = ((($$1512607)) + -1|0);
      HEAP8[$305>>0] = 48;
      $306 = $305;
      $307 = (($301) - ($306))|0;
      $308 = ($307|0)<(2);
      if ($308) {
       $$1512607 = $305;
      } else {
       $$1512$lcssa = $305;
       break;
      }
     }
    } else {
     $$1512$lcssa = $300;
    }
    $309 = $$5519$ph >> 31;
    $310 = $309 & 2;
    $311 = (($310) + 43)|0;
    $312 = $311&255;
    $313 = ((($$1512$lcssa)) + -1|0);
    HEAP8[$313>>0] = $312;
    $314 = $$1480&255;
    $315 = ((($$1512$lcssa)) + -2|0);
    HEAP8[$315>>0] = $314;
    $316 = $315;
    $317 = (($301) - ($316))|0;
    $$2513 = $315;$$pn566 = $317;
   }
   $318 = (($$0520) + 1)|0;
   $319 = (($318) + ($$3477))|0;
   $$1526 = (($319) + ($291))|0;
   $320 = (($$1526) + ($$pn566))|0;
   _pad_674($0,32,$2,$320,$4);
   _out($0,$$0521,$$0520);
   $321 = $4 ^ 65536;
   _pad_674($0,48,$2,$320,$321);
   if ($293) {
    $322 = ($$9$ph>>>0)>($$556>>>0);
    $$0496$$9 = $322 ? $$556 : $$9$ph;
    $323 = ((($8)) + 9|0);
    $324 = $323;
    $325 = ((($8)) + 8|0);
    $$5493597 = $$0496$$9;
    while(1) {
     $326 = HEAP32[$$5493597>>2]|0;
     $327 = (_fmt_u($326,0,$323)|0);
     $328 = ($$5493597|0)==($$0496$$9|0);
     if ($328) {
      $334 = ($327|0)==($323|0);
      if ($334) {
       HEAP8[$325>>0] = 48;
       $$1465 = $325;
      } else {
       $$1465 = $327;
      }
     } else {
      $329 = ($327>>>0)>($8>>>0);
      if ($329) {
       $330 = $327;
       $331 = (($330) - ($9))|0;
       _memset(($8|0),48,($331|0))|0;
       $$0464594 = $327;
       while(1) {
        $332 = ((($$0464594)) + -1|0);
        $333 = ($332>>>0)>($8>>>0);
        if ($333) {
         $$0464594 = $332;
        } else {
         $$1465 = $332;
         break;
        }
       }
      } else {
       $$1465 = $327;
      }
     }
     $335 = $$1465;
     $336 = (($324) - ($335))|0;
     _out($0,$$1465,$336);
     $337 = ((($$5493597)) + 4|0);
     $338 = ($337>>>0)>($$556>>>0);
     if ($338) {
      break;
     } else {
      $$5493597 = $337;
     }
    }
    $339 = ($289|0)==(0);
    if (!($339)) {
     _out($0,11917,1);
    }
    $340 = ($337>>>0)<($$7505>>>0);
    $341 = ($$3477|0)>(0);
    $342 = $340 & $341;
    if ($342) {
     $$4478590 = $$3477;$$6494589 = $337;
     while(1) {
      $343 = HEAP32[$$6494589>>2]|0;
      $344 = (_fmt_u($343,0,$323)|0);
      $345 = ($344>>>0)>($8>>>0);
      if ($345) {
       $346 = $344;
       $347 = (($346) - ($9))|0;
       _memset(($8|0),48,($347|0))|0;
       $$0463584 = $344;
       while(1) {
        $348 = ((($$0463584)) + -1|0);
        $349 = ($348>>>0)>($8>>>0);
        if ($349) {
         $$0463584 = $348;
        } else {
         $$0463$lcssa = $348;
         break;
        }
       }
      } else {
       $$0463$lcssa = $344;
      }
      $350 = ($$4478590|0)<(9);
      $351 = $350 ? $$4478590 : 9;
      _out($0,$$0463$lcssa,$351);
      $352 = ((($$6494589)) + 4|0);
      $353 = (($$4478590) + -9)|0;
      $354 = ($352>>>0)<($$7505>>>0);
      $355 = ($$4478590|0)>(9);
      $356 = $354 & $355;
      if ($356) {
       $$4478590 = $353;$$6494589 = $352;
      } else {
       $$4478$lcssa = $353;
       break;
      }
     }
    } else {
     $$4478$lcssa = $$3477;
    }
    $357 = (($$4478$lcssa) + 9)|0;
    _pad_674($0,48,$357,9,0);
   } else {
    $358 = ((($$9$ph)) + 4|0);
    $$7505$ = $$lcssa673 ? $$7505 : $358;
    $359 = ($$3477|0)>(-1);
    if ($359) {
     $360 = ((($8)) + 9|0);
     $361 = ($$pre$phi690Z2D|0)==(0);
     $362 = $360;
     $363 = (0 - ($9))|0;
     $364 = ((($8)) + 8|0);
     $$5602 = $$3477;$$7495601 = $$9$ph;
     while(1) {
      $365 = HEAP32[$$7495601>>2]|0;
      $366 = (_fmt_u($365,0,$360)|0);
      $367 = ($366|0)==($360|0);
      if ($367) {
       HEAP8[$364>>0] = 48;
       $$0 = $364;
      } else {
       $$0 = $366;
      }
      $368 = ($$7495601|0)==($$9$ph|0);
      do {
       if ($368) {
        $372 = ((($$0)) + 1|0);
        _out($0,$$0,1);
        $373 = ($$5602|0)<(1);
        $or$cond554 = $361 & $373;
        if ($or$cond554) {
         $$2 = $372;
         break;
        }
        _out($0,11917,1);
        $$2 = $372;
       } else {
        $369 = ($$0>>>0)>($8>>>0);
        if (!($369)) {
         $$2 = $$0;
         break;
        }
        $scevgep684 = (($$0) + ($363)|0);
        $scevgep684685 = $scevgep684;
        _memset(($8|0),48,($scevgep684685|0))|0;
        $$1598 = $$0;
        while(1) {
         $370 = ((($$1598)) + -1|0);
         $371 = ($370>>>0)>($8>>>0);
         if ($371) {
          $$1598 = $370;
         } else {
          $$2 = $370;
          break;
         }
        }
       }
      } while(0);
      $374 = $$2;
      $375 = (($362) - ($374))|0;
      $376 = ($$5602|0)>($375|0);
      $377 = $376 ? $375 : $$5602;
      _out($0,$$2,$377);
      $378 = (($$5602) - ($375))|0;
      $379 = ((($$7495601)) + 4|0);
      $380 = ($379>>>0)<($$7505$>>>0);
      $381 = ($378|0)>(-1);
      $382 = $380 & $381;
      if ($382) {
       $$5602 = $378;$$7495601 = $379;
      } else {
       $$5$lcssa = $378;
       break;
      }
     }
    } else {
     $$5$lcssa = $$3477;
    }
    $383 = (($$5$lcssa) + 18)|0;
    _pad_674($0,48,$383,18,0);
    $384 = $11;
    $385 = $$2513;
    $386 = (($384) - ($385))|0;
    _out($0,$$2513,$386);
   }
   $387 = $4 ^ 8192;
   _pad_674($0,32,$2,$320,$387);
   $$sink562 = $320;
  } else {
   $27 = $5 & 32;
   $28 = ($27|0)!=(0);
   $29 = $28 ? 11885 : 11889;
   $30 = ($$0471 != $$0471) | (0.0 != 0.0);
   $31 = $28 ? 11893 : 11897;
   $$0510 = $30 ? $31 : $29;
   $32 = (($$0520) + 3)|0;
   $33 = $4 & -65537;
   _pad_674($0,32,$2,$32,$33);
   _out($0,$$0521,$$0520);
   _out($0,$$0510,3);
   $34 = $4 ^ 8192;
   _pad_674($0,32,$2,$32,$34);
   $$sink562 = $32;
  }
 } while(0);
 $388 = ($$sink562|0)<($2|0);
 $$555 = $388 ? $2 : $$sink562;
 STACKTOP = sp;return ($$555|0);
}
function ___DOUBLE_BITS_675($0) {
 $0 = +$0;
 var $1 = 0, $2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 HEAPF64[tempDoublePtr>>3] = $0;$1 = HEAP32[tempDoublePtr>>2]|0;
 $2 = HEAP32[tempDoublePtr+4>>2]|0;
 tempRet0 = ($2);
 return ($1|0);
}
function _frexpl($0,$1) {
 $0 = +$0;
 $1 = $1|0;
 var $2 = 0.0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = (+_frexp($0,$1));
 return (+$2);
}
function _frexp($0,$1) {
 $0 = +$0;
 $1 = $1|0;
 var $$0 = 0.0, $$016 = 0.0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0.0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0.0, $9 = 0.0, $storemerge = 0, $trunc$clear = 0, label = 0;
 var sp = 0;
 sp = STACKTOP;
 HEAPF64[tempDoublePtr>>3] = $0;$2 = HEAP32[tempDoublePtr>>2]|0;
 $3 = HEAP32[tempDoublePtr+4>>2]|0;
 $4 = (_bitshift64Lshr(($2|0),($3|0),52)|0);
 $5 = tempRet0;
 $6 = $4&65535;
 $trunc$clear = $6 & 2047;
 switch ($trunc$clear<<16>>16) {
 case 0:  {
  $7 = $0 != 0.0;
  if ($7) {
   $8 = $0 * 1.8446744073709552E+19;
   $9 = (+_frexp($8,$1));
   $10 = HEAP32[$1>>2]|0;
   $11 = (($10) + -64)|0;
   $$016 = $9;$storemerge = $11;
  } else {
   $$016 = $0;$storemerge = 0;
  }
  HEAP32[$1>>2] = $storemerge;
  $$0 = $$016;
  break;
 }
 case 2047:  {
  $$0 = $0;
  break;
 }
 default: {
  $12 = $4 & 2047;
  $13 = (($12) + -1022)|0;
  HEAP32[$1>>2] = $13;
  $14 = $3 & -2146435073;
  $15 = $14 | 1071644672;
  HEAP32[tempDoublePtr>>2] = $2;HEAP32[tempDoublePtr+4>>2] = $15;$16 = +HEAPF64[tempDoublePtr>>3];
  $$0 = $16;
 }
 }
 return (+$$0);
}
function _wcrtomb($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$0 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0;
 var $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0;
 var $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $not$ = 0, $or$cond = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ($0|0)==(0|0);
 do {
  if ($3) {
   $$0 = 1;
  } else {
   $4 = ($1>>>0)<(128);
   if ($4) {
    $5 = $1&255;
    HEAP8[$0>>0] = $5;
    $$0 = 1;
    break;
   }
   $6 = (___pthread_self_448()|0);
   $7 = ((($6)) + 188|0);
   $8 = HEAP32[$7>>2]|0;
   $9 = HEAP32[$8>>2]|0;
   $not$ = ($9|0)==(0|0);
   if ($not$) {
    $10 = $1 & -128;
    $11 = ($10|0)==(57216);
    if ($11) {
     $13 = $1&255;
     HEAP8[$0>>0] = $13;
     $$0 = 1;
     break;
    } else {
     $12 = (___errno_location()|0);
     HEAP32[$12>>2] = 84;
     $$0 = -1;
     break;
    }
   }
   $14 = ($1>>>0)<(2048);
   if ($14) {
    $15 = $1 >>> 6;
    $16 = $15 | 192;
    $17 = $16&255;
    $18 = ((($0)) + 1|0);
    HEAP8[$0>>0] = $17;
    $19 = $1 & 63;
    $20 = $19 | 128;
    $21 = $20&255;
    HEAP8[$18>>0] = $21;
    $$0 = 2;
    break;
   }
   $22 = ($1>>>0)<(55296);
   $23 = $1 & -8192;
   $24 = ($23|0)==(57344);
   $or$cond = $22 | $24;
   if ($or$cond) {
    $25 = $1 >>> 12;
    $26 = $25 | 224;
    $27 = $26&255;
    $28 = ((($0)) + 1|0);
    HEAP8[$0>>0] = $27;
    $29 = $1 >>> 6;
    $30 = $29 & 63;
    $31 = $30 | 128;
    $32 = $31&255;
    $33 = ((($0)) + 2|0);
    HEAP8[$28>>0] = $32;
    $34 = $1 & 63;
    $35 = $34 | 128;
    $36 = $35&255;
    HEAP8[$33>>0] = $36;
    $$0 = 3;
    break;
   }
   $37 = (($1) + -65536)|0;
   $38 = ($37>>>0)<(1048576);
   if ($38) {
    $39 = $1 >>> 18;
    $40 = $39 | 240;
    $41 = $40&255;
    $42 = ((($0)) + 1|0);
    HEAP8[$0>>0] = $41;
    $43 = $1 >>> 12;
    $44 = $43 & 63;
    $45 = $44 | 128;
    $46 = $45&255;
    $47 = ((($0)) + 2|0);
    HEAP8[$42>>0] = $46;
    $48 = $1 >>> 6;
    $49 = $48 & 63;
    $50 = $49 | 128;
    $51 = $50&255;
    $52 = ((($0)) + 3|0);
    HEAP8[$47>>0] = $51;
    $53 = $1 & 63;
    $54 = $53 | 128;
    $55 = $54&255;
    HEAP8[$52>>0] = $55;
    $$0 = 4;
    break;
   } else {
    $56 = (___errno_location()|0);
    HEAP32[$56>>2] = 84;
    $$0 = -1;
    break;
   }
  }
 } while(0);
 return ($$0|0);
}
function ___pthread_self_448() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (_pthread_self()|0);
 return ($0|0);
}
function ___pthread_self_105() {
 var $0 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $0 = (_pthread_self()|0);
 return ($0|0);
}
function ___strerror_l($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$012$lcssa = 0, $$01214 = 0, $$016 = 0, $$113 = 0, $$115 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 $$016 = 0;
 while(1) {
  $3 = (11919 + ($$016)|0);
  $4 = HEAP8[$3>>0]|0;
  $5 = $4&255;
  $6 = ($5|0)==($0|0);
  if ($6) {
   label = 2;
   break;
  }
  $7 = (($$016) + 1)|0;
  $8 = ($7|0)==(87);
  if ($8) {
   $$01214 = 12007;$$115 = 87;
   label = 5;
   break;
  } else {
   $$016 = $7;
  }
 }
 if ((label|0) == 2) {
  $2 = ($$016|0)==(0);
  if ($2) {
   $$012$lcssa = 12007;
  } else {
   $$01214 = 12007;$$115 = $$016;
   label = 5;
  }
 }
 if ((label|0) == 5) {
  while(1) {
   label = 0;
   $$113 = $$01214;
   while(1) {
    $9 = HEAP8[$$113>>0]|0;
    $10 = ($9<<24>>24)==(0);
    $11 = ((($$113)) + 1|0);
    if ($10) {
     break;
    } else {
     $$113 = $11;
    }
   }
   $12 = (($$115) + -1)|0;
   $13 = ($12|0)==(0);
   if ($13) {
    $$012$lcssa = $11;
    break;
   } else {
    $$01214 = $11;$$115 = $12;
    label = 5;
   }
  }
 }
 $14 = ((($1)) + 20|0);
 $15 = HEAP32[$14>>2]|0;
 $16 = (___lctrans($$012$lcssa,$15)|0);
 return ($16|0);
}
function ___lctrans($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $2 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = (___lctrans_impl($0,$1)|0);
 return ($2|0);
}
function ___lctrans_impl($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ($1|0)==(0|0);
 if ($2) {
  $$0 = 0;
 } else {
  $3 = HEAP32[$1>>2]|0;
  $4 = ((($1)) + 4|0);
  $5 = HEAP32[$4>>2]|0;
  $6 = (___mo_lookup($3,$5,$0)|0);
  $$0 = $6;
 }
 $7 = ($$0|0)!=(0|0);
 $8 = $7 ? $$0 : $0;
 return ($8|0);
}
function ___mo_lookup($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$ = 0, $$090 = 0, $$094 = 0, $$191 = 0, $$195 = 0, $$4 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0;
 var $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0;
 var $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond102 = 0, $or$cond104 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = HEAP32[$0>>2]|0;
 $4 = (($3) + 1794895138)|0;
 $5 = ((($0)) + 8|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = (_swapc($6,$4)|0);
 $8 = ((($0)) + 12|0);
 $9 = HEAP32[$8>>2]|0;
 $10 = (_swapc($9,$4)|0);
 $11 = ((($0)) + 16|0);
 $12 = HEAP32[$11>>2]|0;
 $13 = (_swapc($12,$4)|0);
 $14 = $1 >>> 2;
 $15 = ($7>>>0)<($14>>>0);
 L1: do {
  if ($15) {
   $16 = $7 << 2;
   $17 = (($1) - ($16))|0;
   $18 = ($10>>>0)<($17>>>0);
   $19 = ($13>>>0)<($17>>>0);
   $or$cond = $18 & $19;
   if ($or$cond) {
    $20 = $13 | $10;
    $21 = $20 & 3;
    $22 = ($21|0)==(0);
    if ($22) {
     $23 = $10 >>> 2;
     $24 = $13 >>> 2;
     $$090 = 0;$$094 = $7;
     while(1) {
      $25 = $$094 >>> 1;
      $26 = (($$090) + ($25))|0;
      $27 = $26 << 1;
      $28 = (($27) + ($23))|0;
      $29 = (($0) + ($28<<2)|0);
      $30 = HEAP32[$29>>2]|0;
      $31 = (_swapc($30,$4)|0);
      $32 = (($28) + 1)|0;
      $33 = (($0) + ($32<<2)|0);
      $34 = HEAP32[$33>>2]|0;
      $35 = (_swapc($34,$4)|0);
      $36 = ($35>>>0)<($1>>>0);
      $37 = (($1) - ($35))|0;
      $38 = ($31>>>0)<($37>>>0);
      $or$cond102 = $36 & $38;
      if (!($or$cond102)) {
       $$4 = 0;
       break L1;
      }
      $39 = (($35) + ($31))|0;
      $40 = (($0) + ($39)|0);
      $41 = HEAP8[$40>>0]|0;
      $42 = ($41<<24>>24)==(0);
      if (!($42)) {
       $$4 = 0;
       break L1;
      }
      $43 = (($0) + ($35)|0);
      $44 = (_strcmp($2,$43)|0);
      $45 = ($44|0)==(0);
      if ($45) {
       break;
      }
      $62 = ($$094|0)==(1);
      $63 = ($44|0)<(0);
      $64 = (($$094) - ($25))|0;
      $$195 = $63 ? $25 : $64;
      $$191 = $63 ? $$090 : $26;
      if ($62) {
       $$4 = 0;
       break L1;
      } else {
       $$090 = $$191;$$094 = $$195;
      }
     }
     $46 = (($27) + ($24))|0;
     $47 = (($0) + ($46<<2)|0);
     $48 = HEAP32[$47>>2]|0;
     $49 = (_swapc($48,$4)|0);
     $50 = (($46) + 1)|0;
     $51 = (($0) + ($50<<2)|0);
     $52 = HEAP32[$51>>2]|0;
     $53 = (_swapc($52,$4)|0);
     $54 = ($53>>>0)<($1>>>0);
     $55 = (($1) - ($53))|0;
     $56 = ($49>>>0)<($55>>>0);
     $or$cond104 = $54 & $56;
     if ($or$cond104) {
      $57 = (($0) + ($53)|0);
      $58 = (($53) + ($49))|0;
      $59 = (($0) + ($58)|0);
      $60 = HEAP8[$59>>0]|0;
      $61 = ($60<<24>>24)==(0);
      $$ = $61 ? $57 : 0;
      $$4 = $$;
     } else {
      $$4 = 0;
     }
    } else {
     $$4 = 0;
    }
   } else {
    $$4 = 0;
   }
  } else {
   $$4 = 0;
  }
 } while(0);
 return ($$4|0);
}
function _swapc($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$ = 0, $2 = 0, $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ($1|0)==(0);
 $3 = (_llvm_bswap_i32(($0|0))|0);
 $$ = $2 ? $0 : $3;
 return ($$|0);
}
function ___fwritex($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$038 = 0, $$042 = 0, $$1 = 0, $$139 = 0, $$141 = 0, $$143 = 0, $$pre = 0, $$pre47 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $20 = 0, $21 = 0;
 var $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ((($2)) + 16|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ($4|0)==(0|0);
 if ($5) {
  $7 = (___towrite($2)|0);
  $8 = ($7|0)==(0);
  if ($8) {
   $$pre = HEAP32[$3>>2]|0;
   $12 = $$pre;
   label = 5;
  } else {
   $$1 = 0;
  }
 } else {
  $6 = $4;
  $12 = $6;
  label = 5;
 }
 L5: do {
  if ((label|0) == 5) {
   $9 = ((($2)) + 20|0);
   $10 = HEAP32[$9>>2]|0;
   $11 = (($12) - ($10))|0;
   $13 = ($11>>>0)<($1>>>0);
   $14 = $10;
   if ($13) {
    $15 = ((($2)) + 36|0);
    $16 = HEAP32[$15>>2]|0;
    $17 = (FUNCTION_TABLE_iiii[$16 & 15]($2,$0,$1)|0);
    $$1 = $17;
    break;
   }
   $18 = ((($2)) + 75|0);
   $19 = HEAP8[$18>>0]|0;
   $20 = ($19<<24>>24)>(-1);
   L10: do {
    if ($20) {
     $$038 = $1;
     while(1) {
      $21 = ($$038|0)==(0);
      if ($21) {
       $$139 = 0;$$141 = $0;$$143 = $1;$31 = $14;
       break L10;
      }
      $22 = (($$038) + -1)|0;
      $23 = (($0) + ($22)|0);
      $24 = HEAP8[$23>>0]|0;
      $25 = ($24<<24>>24)==(10);
      if ($25) {
       break;
      } else {
       $$038 = $22;
      }
     }
     $26 = ((($2)) + 36|0);
     $27 = HEAP32[$26>>2]|0;
     $28 = (FUNCTION_TABLE_iiii[$27 & 15]($2,$0,$$038)|0);
     $29 = ($28>>>0)<($$038>>>0);
     if ($29) {
      $$1 = $28;
      break L5;
     }
     $30 = (($0) + ($$038)|0);
     $$042 = (($1) - ($$038))|0;
     $$pre47 = HEAP32[$9>>2]|0;
     $$139 = $$038;$$141 = $30;$$143 = $$042;$31 = $$pre47;
    } else {
     $$139 = 0;$$141 = $0;$$143 = $1;$31 = $14;
    }
   } while(0);
   _memcpy(($31|0),($$141|0),($$143|0))|0;
   $32 = HEAP32[$9>>2]|0;
   $33 = (($32) + ($$143)|0);
   HEAP32[$9>>2] = $33;
   $34 = (($$139) + ($$143))|0;
   $$1 = $34;
  }
 } while(0);
 return ($$1|0);
}
function ___towrite($0) {
 $0 = $0|0;
 var $$0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0;
 var $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ((($0)) + 74|0);
 $2 = HEAP8[$1>>0]|0;
 $3 = $2 << 24 >> 24;
 $4 = (($3) + 255)|0;
 $5 = $4 | $3;
 $6 = $5&255;
 HEAP8[$1>>0] = $6;
 $7 = HEAP32[$0>>2]|0;
 $8 = $7 & 8;
 $9 = ($8|0)==(0);
 if ($9) {
  $11 = ((($0)) + 8|0);
  HEAP32[$11>>2] = 0;
  $12 = ((($0)) + 4|0);
  HEAP32[$12>>2] = 0;
  $13 = ((($0)) + 44|0);
  $14 = HEAP32[$13>>2]|0;
  $15 = ((($0)) + 28|0);
  HEAP32[$15>>2] = $14;
  $16 = ((($0)) + 20|0);
  HEAP32[$16>>2] = $14;
  $17 = ((($0)) + 48|0);
  $18 = HEAP32[$17>>2]|0;
  $19 = (($14) + ($18)|0);
  $20 = ((($0)) + 16|0);
  HEAP32[$20>>2] = $19;
  $$0 = 0;
 } else {
  $10 = $7 | 32;
  HEAP32[$0>>2] = $10;
  $$0 = -1;
 }
 return ($$0|0);
}
function _sn_write($0,$1,$2) {
 $0 = $0|0;
 $1 = $1|0;
 $2 = $2|0;
 var $$ = 0, $10 = 0, $11 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $3 = ((($0)) + 16|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ((($0)) + 20|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = $6;
 $8 = (($4) - ($7))|0;
 $9 = ($8>>>0)>($2>>>0);
 $$ = $9 ? $2 : $8;
 _memcpy(($6|0),($1|0),($$|0))|0;
 $10 = HEAP32[$5>>2]|0;
 $11 = (($10) + ($$)|0);
 HEAP32[$5>>2] = $11;
 return ($2|0);
}
function _strlen($0) {
 $0 = $0|0;
 var $$0 = 0, $$015$lcssa = 0, $$01519 = 0, $$1$lcssa = 0, $$pn = 0, $$pre = 0, $$sink = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0;
 var $21 = 0, $22 = 0, $23 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = $0;
 $2 = $1 & 3;
 $3 = ($2|0)==(0);
 L1: do {
  if ($3) {
   $$015$lcssa = $0;
   label = 4;
  } else {
   $$01519 = $0;$23 = $1;
   while(1) {
    $4 = HEAP8[$$01519>>0]|0;
    $5 = ($4<<24>>24)==(0);
    if ($5) {
     $$sink = $23;
     break L1;
    }
    $6 = ((($$01519)) + 1|0);
    $7 = $6;
    $8 = $7 & 3;
    $9 = ($8|0)==(0);
    if ($9) {
     $$015$lcssa = $6;
     label = 4;
     break;
    } else {
     $$01519 = $6;$23 = $7;
    }
   }
  }
 } while(0);
 if ((label|0) == 4) {
  $$0 = $$015$lcssa;
  while(1) {
   $10 = HEAP32[$$0>>2]|0;
   $11 = (($10) + -16843009)|0;
   $12 = $10 & -2139062144;
   $13 = $12 ^ -2139062144;
   $14 = $13 & $11;
   $15 = ($14|0)==(0);
   $16 = ((($$0)) + 4|0);
   if ($15) {
    $$0 = $16;
   } else {
    break;
   }
  }
  $17 = $10&255;
  $18 = ($17<<24>>24)==(0);
  if ($18) {
   $$1$lcssa = $$0;
  } else {
   $$pn = $$0;
   while(1) {
    $19 = ((($$pn)) + 1|0);
    $$pre = HEAP8[$19>>0]|0;
    $20 = ($$pre<<24>>24)==(0);
    if ($20) {
     $$1$lcssa = $19;
     break;
    } else {
     $$pn = $19;
    }
   }
  }
  $21 = $$1$lcssa;
  $$sink = $21;
 }
 $22 = (($$sink) - ($1))|0;
 return ($22|0);
}
function _strchr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = (___strchrnul($0,$1)|0);
 $3 = HEAP8[$2>>0]|0;
 $4 = $1&255;
 $5 = ($3<<24>>24)==($4<<24>>24);
 $6 = $5 ? $2 : 0;
 return ($6|0);
}
function ___strchrnul($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $$029$lcssa = 0, $$02936 = 0, $$030$lcssa = 0, $$03039 = 0, $$1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0;
 var $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0;
 var $41 = 0, $42 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond33 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = $1 & 255;
 $3 = ($2|0)==(0);
 L1: do {
  if ($3) {
   $8 = (_strlen($0)|0);
   $9 = (($0) + ($8)|0);
   $$0 = $9;
  } else {
   $4 = $0;
   $5 = $4 & 3;
   $6 = ($5|0)==(0);
   if ($6) {
    $$030$lcssa = $0;
   } else {
    $7 = $1&255;
    $$03039 = $0;
    while(1) {
     $10 = HEAP8[$$03039>>0]|0;
     $11 = ($10<<24>>24)==(0);
     $12 = ($10<<24>>24)==($7<<24>>24);
     $or$cond = $11 | $12;
     if ($or$cond) {
      $$0 = $$03039;
      break L1;
     }
     $13 = ((($$03039)) + 1|0);
     $14 = $13;
     $15 = $14 & 3;
     $16 = ($15|0)==(0);
     if ($16) {
      $$030$lcssa = $13;
      break;
     } else {
      $$03039 = $13;
     }
    }
   }
   $17 = Math_imul($2, 16843009)|0;
   $18 = HEAP32[$$030$lcssa>>2]|0;
   $19 = (($18) + -16843009)|0;
   $20 = $18 & -2139062144;
   $21 = $20 ^ -2139062144;
   $22 = $21 & $19;
   $23 = ($22|0)==(0);
   L10: do {
    if ($23) {
     $$02936 = $$030$lcssa;$25 = $18;
     while(1) {
      $24 = $25 ^ $17;
      $26 = (($24) + -16843009)|0;
      $27 = $24 & -2139062144;
      $28 = $27 ^ -2139062144;
      $29 = $28 & $26;
      $30 = ($29|0)==(0);
      if (!($30)) {
       $$029$lcssa = $$02936;
       break L10;
      }
      $31 = ((($$02936)) + 4|0);
      $32 = HEAP32[$31>>2]|0;
      $33 = (($32) + -16843009)|0;
      $34 = $32 & -2139062144;
      $35 = $34 ^ -2139062144;
      $36 = $35 & $33;
      $37 = ($36|0)==(0);
      if ($37) {
       $$02936 = $31;$25 = $32;
      } else {
       $$029$lcssa = $31;
       break;
      }
     }
    } else {
     $$029$lcssa = $$030$lcssa;
    }
   } while(0);
   $38 = $1&255;
   $$1 = $$029$lcssa;
   while(1) {
    $39 = HEAP8[$$1>>0]|0;
    $40 = ($39<<24>>24)==(0);
    $41 = ($39<<24>>24)==($38<<24>>24);
    $or$cond33 = $40 | $41;
    $42 = ((($$1)) + 1|0);
    if ($or$cond33) {
     $$0 = $$1;
     break;
    } else {
     $$1 = $42;
    }
   }
  }
 } while(0);
 return ($$0|0);
}
function _strcpy($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 (___stpcpy($0,$1)|0);
 return ($0|0);
}
function ___stpcpy($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0$lcssa = 0, $$025$lcssa = 0, $$02536 = 0, $$026$lcssa = 0, $$02642 = 0, $$027$lcssa = 0, $$02741 = 0, $$029 = 0, $$037 = 0, $$1$ph = 0, $$128$ph = 0, $$12834 = 0, $$135 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0;
 var $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0;
 var $35 = 0, $36 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = $1;
 $3 = $0;
 $4 = $2 ^ $3;
 $5 = $4 & 3;
 $6 = ($5|0)==(0);
 L1: do {
  if ($6) {
   $7 = $2 & 3;
   $8 = ($7|0)==(0);
   if ($8) {
    $$026$lcssa = $1;$$027$lcssa = $0;
   } else {
    $$02642 = $1;$$02741 = $0;
    while(1) {
     $9 = HEAP8[$$02642>>0]|0;
     HEAP8[$$02741>>0] = $9;
     $10 = ($9<<24>>24)==(0);
     if ($10) {
      $$029 = $$02741;
      break L1;
     }
     $11 = ((($$02642)) + 1|0);
     $12 = ((($$02741)) + 1|0);
     $13 = $11;
     $14 = $13 & 3;
     $15 = ($14|0)==(0);
     if ($15) {
      $$026$lcssa = $11;$$027$lcssa = $12;
      break;
     } else {
      $$02642 = $11;$$02741 = $12;
     }
    }
   }
   $16 = HEAP32[$$026$lcssa>>2]|0;
   $17 = (($16) + -16843009)|0;
   $18 = $16 & -2139062144;
   $19 = $18 ^ -2139062144;
   $20 = $19 & $17;
   $21 = ($20|0)==(0);
   if ($21) {
    $$02536 = $$027$lcssa;$$037 = $$026$lcssa;$24 = $16;
    while(1) {
     $22 = ((($$037)) + 4|0);
     $23 = ((($$02536)) + 4|0);
     HEAP32[$$02536>>2] = $24;
     $25 = HEAP32[$22>>2]|0;
     $26 = (($25) + -16843009)|0;
     $27 = $25 & -2139062144;
     $28 = $27 ^ -2139062144;
     $29 = $28 & $26;
     $30 = ($29|0)==(0);
     if ($30) {
      $$02536 = $23;$$037 = $22;$24 = $25;
     } else {
      $$0$lcssa = $22;$$025$lcssa = $23;
      break;
     }
    }
   } else {
    $$0$lcssa = $$026$lcssa;$$025$lcssa = $$027$lcssa;
   }
   $$1$ph = $$0$lcssa;$$128$ph = $$025$lcssa;
   label = 8;
  } else {
   $$1$ph = $1;$$128$ph = $0;
   label = 8;
  }
 } while(0);
 if ((label|0) == 8) {
  $31 = HEAP8[$$1$ph>>0]|0;
  HEAP8[$$128$ph>>0] = $31;
  $32 = ($31<<24>>24)==(0);
  if ($32) {
   $$029 = $$128$ph;
  } else {
   $$12834 = $$128$ph;$$135 = $$1$ph;
   while(1) {
    $33 = ((($$135)) + 1|0);
    $34 = ((($$12834)) + 1|0);
    $35 = HEAP8[$33>>0]|0;
    HEAP8[$34>>0] = $35;
    $36 = ($35<<24>>24)==(0);
    if ($36) {
     $$029 = $34;
     break;
    } else {
     $$12834 = $34;$$135 = $33;
    }
   }
  }
 }
 return ($$029|0);
}
function ___ofl_lock() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 ___lock((16420|0));
 return (16428|0);
}
function ___ofl_unlock() {
 var label = 0, sp = 0;
 sp = STACKTOP;
 ___unlock((16420|0));
 return;
}
function _fflush($0) {
 $0 = $0|0;
 var $$0 = 0, $$023 = 0, $$02325 = 0, $$02327 = 0, $$024$lcssa = 0, $$02426 = 0, $$1 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0;
 var $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $phitmp = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ($0|0)==(0|0);
 do {
  if ($1) {
   $8 = HEAP32[841]|0;
   $9 = ($8|0)==(0|0);
   if ($9) {
    $29 = 0;
   } else {
    $10 = HEAP32[841]|0;
    $11 = (_fflush($10)|0);
    $29 = $11;
   }
   $12 = (___ofl_lock()|0);
   $$02325 = HEAP32[$12>>2]|0;
   $13 = ($$02325|0)==(0|0);
   if ($13) {
    $$024$lcssa = $29;
   } else {
    $$02327 = $$02325;$$02426 = $29;
    while(1) {
     $14 = ((($$02327)) + 76|0);
     $15 = HEAP32[$14>>2]|0;
     $16 = ($15|0)>(-1);
     if ($16) {
      $17 = (___lockfile($$02327)|0);
      $26 = $17;
     } else {
      $26 = 0;
     }
     $18 = ((($$02327)) + 20|0);
     $19 = HEAP32[$18>>2]|0;
     $20 = ((($$02327)) + 28|0);
     $21 = HEAP32[$20>>2]|0;
     $22 = ($19>>>0)>($21>>>0);
     if ($22) {
      $23 = (___fflush_unlocked($$02327)|0);
      $24 = $23 | $$02426;
      $$1 = $24;
     } else {
      $$1 = $$02426;
     }
     $25 = ($26|0)==(0);
     if (!($25)) {
      ___unlockfile($$02327);
     }
     $27 = ((($$02327)) + 56|0);
     $$023 = HEAP32[$27>>2]|0;
     $28 = ($$023|0)==(0|0);
     if ($28) {
      $$024$lcssa = $$1;
      break;
     } else {
      $$02327 = $$023;$$02426 = $$1;
     }
    }
   }
   ___ofl_unlock();
   $$0 = $$024$lcssa;
  } else {
   $2 = ((($0)) + 76|0);
   $3 = HEAP32[$2>>2]|0;
   $4 = ($3|0)>(-1);
   if (!($4)) {
    $5 = (___fflush_unlocked($0)|0);
    $$0 = $5;
    break;
   }
   $6 = (___lockfile($0)|0);
   $phitmp = ($6|0)==(0);
   $7 = (___fflush_unlocked($0)|0);
   if ($phitmp) {
    $$0 = $7;
   } else {
    ___unlockfile($0);
    $$0 = $7;
   }
  }
 } while(0);
 return ($$0|0);
}
function ___fflush_unlocked($0) {
 $0 = $0|0;
 var $$0 = 0, $1 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0;
 var $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ((($0)) + 20|0);
 $2 = HEAP32[$1>>2]|0;
 $3 = ((($0)) + 28|0);
 $4 = HEAP32[$3>>2]|0;
 $5 = ($2>>>0)>($4>>>0);
 if ($5) {
  $6 = ((($0)) + 36|0);
  $7 = HEAP32[$6>>2]|0;
  (FUNCTION_TABLE_iiii[$7 & 15]($0,0,0)|0);
  $8 = HEAP32[$1>>2]|0;
  $9 = ($8|0)==(0|0);
  if ($9) {
   $$0 = -1;
  } else {
   label = 3;
  }
 } else {
  label = 3;
 }
 if ((label|0) == 3) {
  $10 = ((($0)) + 4|0);
  $11 = HEAP32[$10>>2]|0;
  $12 = ((($0)) + 8|0);
  $13 = HEAP32[$12>>2]|0;
  $14 = ($11>>>0)<($13>>>0);
  if ($14) {
   $15 = $11;
   $16 = $13;
   $17 = (($15) - ($16))|0;
   $18 = ((($0)) + 40|0);
   $19 = HEAP32[$18>>2]|0;
   (FUNCTION_TABLE_iiii[$19 & 15]($0,$17,1)|0);
  }
  $20 = ((($0)) + 16|0);
  HEAP32[$20>>2] = 0;
  HEAP32[$3>>2] = 0;
  HEAP32[$1>>2] = 0;
  HEAP32[$12>>2] = 0;
  HEAP32[$10>>2] = 0;
  $$0 = 0;
 }
 return ($$0|0);
}
function _strstr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0;
 var $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP8[$1>>0]|0;
 $3 = ($2<<24>>24)==(0);
 do {
  if ($3) {
   $$0 = $0;
  } else {
   $4 = $2 << 24 >> 24;
   $5 = (_strchr($0,$4)|0);
   $6 = ($5|0)==(0|0);
   if ($6) {
    $$0 = 0;
   } else {
    $7 = ((($1)) + 1|0);
    $8 = HEAP8[$7>>0]|0;
    $9 = ($8<<24>>24)==(0);
    if ($9) {
     $$0 = $5;
    } else {
     $10 = ((($5)) + 1|0);
     $11 = HEAP8[$10>>0]|0;
     $12 = ($11<<24>>24)==(0);
     if ($12) {
      $$0 = 0;
     } else {
      $13 = ((($1)) + 2|0);
      $14 = HEAP8[$13>>0]|0;
      $15 = ($14<<24>>24)==(0);
      if ($15) {
       $16 = (_twobyte_strstr($5,$1)|0);
       $$0 = $16;
       break;
      }
      $17 = ((($5)) + 2|0);
      $18 = HEAP8[$17>>0]|0;
      $19 = ($18<<24>>24)==(0);
      if ($19) {
       $$0 = 0;
      } else {
       $20 = ((($1)) + 3|0);
       $21 = HEAP8[$20>>0]|0;
       $22 = ($21<<24>>24)==(0);
       if ($22) {
        $23 = (_threebyte_strstr($5,$1)|0);
        $$0 = $23;
        break;
       }
       $24 = ((($5)) + 3|0);
       $25 = HEAP8[$24>>0]|0;
       $26 = ($25<<24>>24)==(0);
       if ($26) {
        $$0 = 0;
       } else {
        $27 = ((($1)) + 4|0);
        $28 = HEAP8[$27>>0]|0;
        $29 = ($28<<24>>24)==(0);
        if ($29) {
         $30 = (_fourbyte_strstr($5,$1)|0);
         $$0 = $30;
         break;
        } else {
         $31 = (_twoway_strstr($5,$1)|0);
         $$0 = $31;
         break;
        }
       }
      }
     }
    }
   }
  }
 } while(0);
 return ($$0|0);
}
function _twobyte_strstr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$lcssa = 0, $$sink = 0, $$sink$in = 0, $$sink$masked = 0, $$sink17$sink = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0;
 var label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP8[$1>>0]|0;
 $3 = $2&255;
 $4 = $3 << 8;
 $5 = ((($1)) + 1|0);
 $6 = HEAP8[$5>>0]|0;
 $7 = $6&255;
 $8 = $4 | $7;
 $9 = HEAP8[$0>>0]|0;
 $10 = $9&255;
 $$sink$in = $10;$$sink17$sink = $0;
 while(1) {
  $11 = ((($$sink17$sink)) + 1|0);
  $12 = HEAP8[$11>>0]|0;
  $13 = ($12<<24>>24)==(0);
  if ($13) {
   $$lcssa = 0;
   break;
  }
  $$sink = $$sink$in << 8;
  $14 = $12&255;
  $$sink$masked = $$sink & 65280;
  $15 = $14 | $$sink$masked;
  $16 = ($15|0)==($8|0);
  if ($16) {
   $$lcssa = $$sink17$sink;
   break;
  } else {
   $$sink$in = $15;$$sink17$sink = $11;
  }
 }
 return ($$lcssa|0);
}
function _threebyte_strstr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$016$lcssa = 0, $$01619 = 0, $$020 = 0, $$lcssa = 0, $$not = 0, $$not17 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0;
 var $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $4 = 0, $5 = 0, $6 = 0;
 var $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond18 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP8[$1>>0]|0;
 $3 = $2&255;
 $4 = $3 << 24;
 $5 = ((($1)) + 1|0);
 $6 = HEAP8[$5>>0]|0;
 $7 = $6&255;
 $8 = $7 << 16;
 $9 = $8 | $4;
 $10 = ((($1)) + 2|0);
 $11 = HEAP8[$10>>0]|0;
 $12 = $11&255;
 $13 = $12 << 8;
 $14 = $9 | $13;
 $15 = HEAP8[$0>>0]|0;
 $16 = $15&255;
 $17 = $16 << 24;
 $18 = ((($0)) + 1|0);
 $19 = HEAP8[$18>>0]|0;
 $20 = $19&255;
 $21 = $20 << 16;
 $22 = $21 | $17;
 $23 = ((($0)) + 2|0);
 $24 = HEAP8[$23>>0]|0;
 $25 = $24&255;
 $26 = $25 << 8;
 $27 = $22 | $26;
 $28 = ($24<<24>>24)!=(0);
 $$not17 = $28 ^ 1;
 $29 = ($27|0)==($14|0);
 $or$cond18 = $29 | $$not17;
 if ($or$cond18) {
  $$016$lcssa = $23;$$lcssa = $28;
 } else {
  $$01619 = $23;$$020 = $27;
  while(1) {
   $30 = ((($$01619)) + 1|0);
   $31 = HEAP8[$30>>0]|0;
   $32 = $31&255;
   $33 = $32 | $$020;
   $34 = $33 << 8;
   $35 = ($31<<24>>24)!=(0);
   $$not = $35 ^ 1;
   $36 = ($34|0)==($14|0);
   $or$cond = $36 | $$not;
   if ($or$cond) {
    $$016$lcssa = $30;$$lcssa = $35;
    break;
   } else {
    $$01619 = $30;$$020 = $34;
   }
  }
 }
 $37 = ((($$016$lcssa)) + -2|0);
 $38 = $$lcssa ? $37 : 0;
 return ($38|0);
}
function _fourbyte_strstr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$lcssa = 0, $$not = 0, $$not22 = 0, $$sink21$lcssa = 0, $$sink2124 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0;
 var $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $or$cond = 0, $or$cond23 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP8[$1>>0]|0;
 $3 = $2&255;
 $4 = $3 << 24;
 $5 = ((($1)) + 1|0);
 $6 = HEAP8[$5>>0]|0;
 $7 = $6&255;
 $8 = $7 << 16;
 $9 = $8 | $4;
 $10 = ((($1)) + 2|0);
 $11 = HEAP8[$10>>0]|0;
 $12 = $11&255;
 $13 = $12 << 8;
 $14 = $9 | $13;
 $15 = ((($1)) + 3|0);
 $16 = HEAP8[$15>>0]|0;
 $17 = $16&255;
 $18 = $14 | $17;
 $19 = HEAP8[$0>>0]|0;
 $20 = $19&255;
 $21 = $20 << 24;
 $22 = ((($0)) + 1|0);
 $23 = HEAP8[$22>>0]|0;
 $24 = $23&255;
 $25 = $24 << 16;
 $26 = $25 | $21;
 $27 = ((($0)) + 2|0);
 $28 = HEAP8[$27>>0]|0;
 $29 = $28&255;
 $30 = $29 << 8;
 $31 = $26 | $30;
 $32 = ((($0)) + 3|0);
 $33 = HEAP8[$32>>0]|0;
 $34 = $33&255;
 $35 = $34 | $31;
 $36 = ($33<<24>>24)!=(0);
 $$not22 = $36 ^ 1;
 $37 = ($35|0)==($18|0);
 $or$cond23 = $37 | $$not22;
 if ($or$cond23) {
  $$lcssa = $36;$$sink21$lcssa = $32;
 } else {
  $$sink2124 = $32;$39 = $35;
  while(1) {
   $38 = $39 << 8;
   $40 = ((($$sink2124)) + 1|0);
   $41 = HEAP8[$40>>0]|0;
   $42 = $41&255;
   $43 = $42 | $38;
   $44 = ($41<<24>>24)!=(0);
   $$not = $44 ^ 1;
   $45 = ($43|0)==($18|0);
   $or$cond = $45 | $$not;
   if ($or$cond) {
    $$lcssa = $44;$$sink21$lcssa = $40;
    break;
   } else {
    $$sink2124 = $40;$39 = $43;
   }
  }
 }
 $46 = ((($$sink21$lcssa)) + -3|0);
 $47 = $$lcssa ? $46 : 0;
 return ($47|0);
}
function _twoway_strstr($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0166 = 0, $$0168 = 0, $$0169 = 0, $$0169$be = 0, $$0170 = 0, $$0175$ph$ph$lcssa220 = 0, $$0175$ph$ph$lcssa220323 = 0, $$0175$ph$ph256 = 0, $$0179244 = 0, $$0183$ph200$ph255 = 0, $$0183$ph200250 = 0, $$0183$ph262 = 0, $$0185$ph$lcssa = 0, $$0185$ph$lcssa322 = 0, $$0185$ph261 = 0, $$0187$lcssa320321 = 0, $$0187266 = 0, $$1176$$0175 = 0, $$1176$ph$ph$lcssa211 = 0, $$1176$ph$ph235 = 0;
 var $$1180224 = 0, $$1184$ph196$ph234 = 0, $$1184$ph196229 = 0, $$1184$ph241 = 0, $$1186$$0185 = 0, $$1186$$0185$ = 0, $$1186$ph$lcssa = 0, $$1186$ph240 = 0, $$2181 = 0, $$2181$sink = 0, $$3 = 0, $$3173 = 0, $$3178 = 0, $$3182223 = 0, $$4 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0;
 var $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0;
 var $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0;
 var $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0;
 var $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0, $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0;
 var $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0, $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0;
 var $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0, $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0;
 var $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0, $99 = 0, $cond = 0, $cond191 = 0, $cond191222 = 0, $cond265 = 0, $div = 0, $div188 = 0, $or$cond = 0, $or$cond190 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 1056|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(1056|0);
 $2 = sp + 1024|0;
 $3 = sp;
 ;HEAP32[$2>>2]=0|0;HEAP32[$2+4>>2]=0|0;HEAP32[$2+8>>2]=0|0;HEAP32[$2+12>>2]=0|0;HEAP32[$2+16>>2]=0|0;HEAP32[$2+20>>2]=0|0;HEAP32[$2+24>>2]=0|0;HEAP32[$2+28>>2]=0|0;
 $4 = HEAP8[$1>>0]|0;
 $cond265 = ($4<<24>>24)==(0);
 L1: do {
  if ($cond265) {
   $$0175$ph$ph$lcssa220323 = 1;$$0185$ph$lcssa322 = -1;$$0187$lcssa320321 = 0;$$1176$ph$ph$lcssa211 = 1;$$1186$ph$lcssa = -1;
   label = 27;
  } else {
   $5 = $4&255;
   $$0187266 = 0;$12 = $4;$20 = $5;
   while(1) {
    $8 = (($0) + ($$0187266)|0);
    $9 = HEAP8[$8>>0]|0;
    $10 = ($9<<24>>24)==(0);
    if ($10) {
     $$3 = 0;
     break L1;
    }
    $11 = $12 & 31;
    $13 = $11&255;
    $14 = 1 << $13;
    $div188 = ($12&255) >>> 5;
    $15 = $div188&255;
    $16 = (($2) + ($15<<2)|0);
    $17 = HEAP32[$16>>2]|0;
    $18 = $17 | $14;
    HEAP32[$16>>2] = $18;
    $7 = (($$0187266) + 1)|0;
    $19 = (($3) + ($20<<2)|0);
    HEAP32[$19>>2] = $7;
    $21 = (($1) + ($7)|0);
    $22 = HEAP8[$21>>0]|0;
    $23 = $22&255;
    $cond = ($22<<24>>24)==(0);
    if ($cond) {
     break;
    } else {
     $$0187266 = $7;$12 = $22;$20 = $23;
    }
   }
   $6 = ($7>>>0)>(1);
   if ($6) {
    $$0183$ph262 = 0;$$0185$ph261 = -1;$129 = 1;
    L7: while(1) {
     $$0175$ph$ph256 = 1;$$0183$ph200$ph255 = $$0183$ph262;$132 = $129;
     while(1) {
      $$0183$ph200250 = $$0183$ph200$ph255;$131 = $132;
      L11: while(1) {
       $$0179244 = 1;$31 = $131;
       while(1) {
        $27 = (($$0179244) + ($$0185$ph261))|0;
        $28 = (($1) + ($27)|0);
        $29 = HEAP8[$28>>0]|0;
        $30 = (($1) + ($31)|0);
        $32 = HEAP8[$30>>0]|0;
        $33 = ($29<<24>>24)==($32<<24>>24);
        if (!($33)) {
         break L11;
        }
        $34 = ($$0179244|0)==($$0175$ph$ph256|0);
        $25 = (($$0179244) + 1)|0;
        if ($34) {
         break;
        }
        $24 = (($25) + ($$0183$ph200250))|0;
        $26 = ($24>>>0)<($7>>>0);
        if ($26) {
         $$0179244 = $25;$31 = $24;
        } else {
         $$0175$ph$ph$lcssa220 = $$0175$ph$ph256;$$0185$ph$lcssa = $$0185$ph261;
         break L7;
        }
       }
       $35 = (($$0175$ph$ph256) + ($$0183$ph200250))|0;
       $36 = (($35) + 1)|0;
       $37 = ($36>>>0)<($7>>>0);
       if ($37) {
        $$0183$ph200250 = $35;$131 = $36;
       } else {
        $$0175$ph$ph$lcssa220 = $$0175$ph$ph256;$$0185$ph$lcssa = $$0185$ph261;
        break L7;
       }
      }
      $38 = ($29&255)>($32&255);
      $39 = (($31) - ($$0185$ph261))|0;
      if (!($38)) {
       break;
      }
      $43 = (($31) + 1)|0;
      $44 = ($43>>>0)<($7>>>0);
      if ($44) {
       $$0175$ph$ph256 = $39;$$0183$ph200$ph255 = $31;$132 = $43;
      } else {
       $$0175$ph$ph$lcssa220 = $39;$$0185$ph$lcssa = $$0185$ph261;
       break L7;
      }
     }
     $40 = (($$0183$ph200250) + 1)|0;
     $41 = (($$0183$ph200250) + 2)|0;
     $42 = ($41>>>0)<($7>>>0);
     if ($42) {
      $$0183$ph262 = $40;$$0185$ph261 = $$0183$ph200250;$129 = $41;
     } else {
      $$0175$ph$ph$lcssa220 = 1;$$0185$ph$lcssa = $$0183$ph200250;
      break;
     }
    }
    if ($6) {
     $$1184$ph241 = 0;$$1186$ph240 = -1;$130 = 1;
     while(1) {
      $$1176$ph$ph235 = 1;$$1184$ph196$ph234 = $$1184$ph241;$134 = $130;
      while(1) {
       $$1184$ph196229 = $$1184$ph196$ph234;$133 = $134;
       L26: while(1) {
        $$1180224 = 1;$52 = $133;
        while(1) {
         $48 = (($$1180224) + ($$1186$ph240))|0;
         $49 = (($1) + ($48)|0);
         $50 = HEAP8[$49>>0]|0;
         $51 = (($1) + ($52)|0);
         $53 = HEAP8[$51>>0]|0;
         $54 = ($50<<24>>24)==($53<<24>>24);
         if (!($54)) {
          break L26;
         }
         $55 = ($$1180224|0)==($$1176$ph$ph235|0);
         $46 = (($$1180224) + 1)|0;
         if ($55) {
          break;
         }
         $45 = (($46) + ($$1184$ph196229))|0;
         $47 = ($45>>>0)<($7>>>0);
         if ($47) {
          $$1180224 = $46;$52 = $45;
         } else {
          $$0175$ph$ph$lcssa220323 = $$0175$ph$ph$lcssa220;$$0185$ph$lcssa322 = $$0185$ph$lcssa;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = $$1176$ph$ph235;$$1186$ph$lcssa = $$1186$ph240;
          label = 27;
          break L1;
         }
        }
        $56 = (($$1176$ph$ph235) + ($$1184$ph196229))|0;
        $57 = (($56) + 1)|0;
        $58 = ($57>>>0)<($7>>>0);
        if ($58) {
         $$1184$ph196229 = $56;$133 = $57;
        } else {
         $$0175$ph$ph$lcssa220323 = $$0175$ph$ph$lcssa220;$$0185$ph$lcssa322 = $$0185$ph$lcssa;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = $$1176$ph$ph235;$$1186$ph$lcssa = $$1186$ph240;
         label = 27;
         break L1;
        }
       }
       $59 = ($50&255)<($53&255);
       $60 = (($52) - ($$1186$ph240))|0;
       if (!($59)) {
        break;
       }
       $64 = (($52) + 1)|0;
       $65 = ($64>>>0)<($7>>>0);
       if ($65) {
        $$1176$ph$ph235 = $60;$$1184$ph196$ph234 = $52;$134 = $64;
       } else {
        $$0175$ph$ph$lcssa220323 = $$0175$ph$ph$lcssa220;$$0185$ph$lcssa322 = $$0185$ph$lcssa;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = $60;$$1186$ph$lcssa = $$1186$ph240;
        label = 27;
        break L1;
       }
      }
      $61 = (($$1184$ph196229) + 1)|0;
      $62 = (($$1184$ph196229) + 2)|0;
      $63 = ($62>>>0)<($7>>>0);
      if ($63) {
       $$1184$ph241 = $61;$$1186$ph240 = $$1184$ph196229;$130 = $62;
      } else {
       $$0175$ph$ph$lcssa220323 = $$0175$ph$ph$lcssa220;$$0185$ph$lcssa322 = $$0185$ph$lcssa;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = 1;$$1186$ph$lcssa = $$1184$ph196229;
       label = 27;
       break;
      }
     }
    } else {
     $$0175$ph$ph$lcssa220323 = $$0175$ph$ph$lcssa220;$$0185$ph$lcssa322 = $$0185$ph$lcssa;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = 1;$$1186$ph$lcssa = -1;
     label = 27;
    }
   } else {
    $$0175$ph$ph$lcssa220323 = 1;$$0185$ph$lcssa322 = -1;$$0187$lcssa320321 = $7;$$1176$ph$ph$lcssa211 = 1;$$1186$ph$lcssa = -1;
    label = 27;
   }
  }
 } while(0);
 L36: do {
  if ((label|0) == 27) {
   $66 = (($$1186$ph$lcssa) + 1)|0;
   $67 = (($$0185$ph$lcssa322) + 1)|0;
   $68 = ($66>>>0)>($67>>>0);
   $$1176$$0175 = $68 ? $$1176$ph$ph$lcssa211 : $$0175$ph$ph$lcssa220323;
   $$1186$$0185 = $68 ? $$1186$ph$lcssa : $$0185$ph$lcssa322;
   $69 = (($1) + ($$1176$$0175)|0);
   $70 = (($$1186$$0185) + 1)|0;
   $71 = (_memcmp($1,$69,$70)|0);
   $72 = ($71|0)==(0);
   if ($72) {
    $77 = (($$0187$lcssa320321) - ($$1176$$0175))|0;
    $$0168 = $77;$$3178 = $$1176$$0175;
   } else {
    $73 = (($$0187$lcssa320321) - ($$1186$$0185))|0;
    $74 = (($73) + -1)|0;
    $75 = ($$1186$$0185>>>0)>($74>>>0);
    $$1186$$0185$ = $75 ? $$1186$$0185 : $74;
    $76 = (($$1186$$0185$) + 1)|0;
    $$0168 = 0;$$3178 = $76;
   }
   $78 = $$0187$lcssa320321 | 63;
   $79 = (($$0187$lcssa320321) + -1)|0;
   $80 = ($$0168|0)!=(0);
   $81 = (($$0187$lcssa320321) - ($$3178))|0;
   $$0166 = $0;$$0169 = 0;$$0170 = $0;
   while(1) {
    $82 = $$0170;
    $83 = $$0166;
    $84 = (($82) - ($83))|0;
    $85 = ($84>>>0)<($$0187$lcssa320321>>>0);
    do {
     if ($85) {
      $86 = (_memchr($$0170,0,$78)|0);
      $87 = ($86|0)==(0|0);
      if ($87) {
       $91 = (($$0170) + ($78)|0);
       $$3173 = $91;
       break;
      } else {
       $88 = $86;
       $89 = (($88) - ($83))|0;
       $90 = ($89>>>0)<($$0187$lcssa320321>>>0);
       if ($90) {
        $$3 = 0;
        break L36;
       } else {
        $$3173 = $86;
        break;
       }
      }
     } else {
      $$3173 = $$0170;
     }
    } while(0);
    $92 = (($$0166) + ($79)|0);
    $93 = HEAP8[$92>>0]|0;
    $div = ($93&255) >>> 5;
    $94 = $div&255;
    $95 = (($2) + ($94<<2)|0);
    $96 = HEAP32[$95>>2]|0;
    $97 = $93 & 31;
    $98 = $97&255;
    $99 = 1 << $98;
    $100 = $99 & $96;
    $101 = ($100|0)==(0);
    L50: do {
     if ($101) {
      $$0169$be = 0;$$2181$sink = $$0187$lcssa320321;
     } else {
      $102 = $93&255;
      $103 = (($3) + ($102<<2)|0);
      $104 = HEAP32[$103>>2]|0;
      $105 = (($$0187$lcssa320321) - ($104))|0;
      $106 = ($105|0)==(0);
      if (!($106)) {
       $107 = ($$0169|0)!=(0);
       $or$cond = $80 & $107;
       $108 = ($105>>>0)<($$3178>>>0);
       $or$cond190 = $or$cond & $108;
       $$2181 = $or$cond190 ? $81 : $105;
       $$0169$be = 0;$$2181$sink = $$2181;
       break;
      }
      $110 = ($70>>>0)>($$0169>>>0);
      $111 = $110 ? $70 : $$0169;
      $112 = (($1) + ($111)|0);
      $113 = HEAP8[$112>>0]|0;
      $cond191222 = ($113<<24>>24)==(0);
      L55: do {
       if ($cond191222) {
        $$4 = $70;
       } else {
        $$3182223 = $111;$117 = $113;
        while(1) {
         $114 = (($$0166) + ($$3182223)|0);
         $115 = HEAP8[$114>>0]|0;
         $116 = ($117<<24>>24)==($115<<24>>24);
         if (!($116)) {
          break;
         }
         $118 = (($$3182223) + 1)|0;
         $119 = (($1) + ($118)|0);
         $120 = HEAP8[$119>>0]|0;
         $cond191 = ($120<<24>>24)==(0);
         if ($cond191) {
          $$4 = $70;
          break L55;
         } else {
          $$3182223 = $118;$117 = $120;
         }
        }
        $121 = (($$3182223) - ($$1186$$0185))|0;
        $$0169$be = 0;$$2181$sink = $121;
        break L50;
       }
      } while(0);
      while(1) {
       $122 = ($$4>>>0)>($$0169>>>0);
       if (!($122)) {
        $$3 = $$0166;
        break L36;
       }
       $123 = (($$4) + -1)|0;
       $124 = (($1) + ($123)|0);
       $125 = HEAP8[$124>>0]|0;
       $126 = (($$0166) + ($123)|0);
       $127 = HEAP8[$126>>0]|0;
       $128 = ($125<<24>>24)==($127<<24>>24);
       if ($128) {
        $$4 = $123;
       } else {
        $$0169$be = $$0168;$$2181$sink = $$3178;
        break;
       }
      }
     }
    } while(0);
    $109 = (($$0166) + ($$2181$sink)|0);
    $$0166 = $109;$$0169 = $$0169$be;$$0170 = $$3173;
   }
  }
 } while(0);
 STACKTOP = sp;return ($$3|0);
}
function _strspn($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $$01925 = 0, $$020 = 0, $$1$lcssa = 0, $$123 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0;
 var $24 = 0, $25 = 0, $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0;
 var $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $div = 0, $div21 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $2 = sp;
 ;HEAP32[$2>>2]=0|0;HEAP32[$2+4>>2]=0|0;HEAP32[$2+8>>2]=0|0;HEAP32[$2+12>>2]=0|0;HEAP32[$2+16>>2]=0|0;HEAP32[$2+20>>2]=0|0;HEAP32[$2+24>>2]=0|0;HEAP32[$2+28>>2]=0|0;
 $3 = HEAP8[$1>>0]|0;
 $4 = ($3<<24>>24)==(0);
 do {
  if ($4) {
   $$0 = 0;
  } else {
   $5 = ((($1)) + 1|0);
   $6 = HEAP8[$5>>0]|0;
   $7 = ($6<<24>>24)==(0);
   if ($7) {
    $$020 = $0;
    while(1) {
     $8 = HEAP8[$$020>>0]|0;
     $9 = ($8<<24>>24)==($3<<24>>24);
     $10 = ((($$020)) + 1|0);
     if ($9) {
      $$020 = $10;
     } else {
      break;
     }
    }
    $11 = $$020;
    $12 = $0;
    $13 = (($11) - ($12))|0;
    $$0 = $13;
    break;
   } else {
    $$01925 = $1;$17 = $3;
   }
   while(1) {
    $16 = $17 & 31;
    $18 = $16&255;
    $19 = 1 << $18;
    $div21 = ($17&255) >>> 5;
    $20 = $div21&255;
    $21 = (($2) + ($20<<2)|0);
    $22 = HEAP32[$21>>2]|0;
    $23 = $22 | $19;
    HEAP32[$21>>2] = $23;
    $24 = ((($$01925)) + 1|0);
    $25 = HEAP8[$24>>0]|0;
    $26 = ($25<<24>>24)==(0);
    if ($26) {
     break;
    } else {
     $$01925 = $24;$17 = $25;
    }
   }
   $14 = HEAP8[$0>>0]|0;
   $15 = ($14<<24>>24)==(0);
   L10: do {
    if ($15) {
     $$1$lcssa = $0;
    } else {
     $$123 = $0;$27 = $14;
     while(1) {
      $div = ($27&255) >>> 5;
      $28 = $div&255;
      $29 = (($2) + ($28<<2)|0);
      $30 = HEAP32[$29>>2]|0;
      $31 = $27 & 31;
      $32 = $31&255;
      $33 = 1 << $32;
      $34 = $30 & $33;
      $35 = ($34|0)==(0);
      if ($35) {
       $$1$lcssa = $$123;
       break L10;
      }
      $36 = ((($$123)) + 1|0);
      $37 = HEAP8[$36>>0]|0;
      $38 = ($37<<24>>24)==(0);
      if ($38) {
       $$1$lcssa = $36;
       break;
      } else {
       $$123 = $36;$27 = $37;
      }
     }
    }
   } while(0);
   $39 = $$1$lcssa;
   $40 = $0;
   $41 = (($39) - ($40))|0;
   $$0 = $41;
  }
 } while(0);
 STACKTOP = sp;return ($$0|0);
}
function _srand($0) {
 $0 = $0|0;
 var $1 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = (($0) + -1)|0;
 $2 = 14080;
 $3 = $2;
 HEAP32[$3>>2] = $1;
 $4 = (($2) + 4)|0;
 $5 = $4;
 HEAP32[$5>>2] = 0;
 return;
}
function _vprintf($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $2 = 0, $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = HEAP32[809]|0;
 $3 = (_vfprintf($2,$0,$1)|0);
 return ($3|0);
}
function _strcspn($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$01824 = 0, $$019$sink = 0, $$01922 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0, $2 = 0, $20 = 0, $21 = 0, $22 = 0, $23 = 0, $24 = 0, $25 = 0;
 var $26 = 0, $27 = 0, $28 = 0, $29 = 0, $3 = 0, $30 = 0, $31 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, $div = 0;
 var $div20 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(32|0);
 $2 = sp;
 $3 = HEAP8[$1>>0]|0;
 $4 = ($3<<24>>24)==(0);
 L1: do {
  if ($4) {
   label = 3;
  } else {
   $5 = ((($1)) + 1|0);
   $6 = HEAP8[$5>>0]|0;
   $7 = ($6<<24>>24)==(0);
   if ($7) {
    label = 3;
   } else {
    ;HEAP32[$2>>2]=0|0;HEAP32[$2+4>>2]=0|0;HEAP32[$2+8>>2]=0|0;HEAP32[$2+12>>2]=0|0;HEAP32[$2+16>>2]=0|0;HEAP32[$2+20>>2]=0|0;HEAP32[$2+24>>2]=0|0;HEAP32[$2+28>>2]=0|0;
    $$01824 = $1;$13 = $3;
    while(1) {
     $12 = $13 & 31;
     $14 = $12&255;
     $15 = 1 << $14;
     $div20 = ($13&255) >>> 5;
     $16 = $div20&255;
     $17 = (($2) + ($16<<2)|0);
     $18 = HEAP32[$17>>2]|0;
     $19 = $18 | $15;
     HEAP32[$17>>2] = $19;
     $20 = ((($$01824)) + 1|0);
     $21 = HEAP8[$20>>0]|0;
     $22 = ($21<<24>>24)==(0);
     if ($22) {
      break;
     } else {
      $$01824 = $20;$13 = $21;
     }
    }
    $10 = HEAP8[$0>>0]|0;
    $11 = ($10<<24>>24)==(0);
    if ($11) {
     $$019$sink = $0;
    } else {
     $$01922 = $0;$23 = $10;
     while(1) {
      $div = ($23&255) >>> 5;
      $24 = $div&255;
      $25 = (($2) + ($24<<2)|0);
      $26 = HEAP32[$25>>2]|0;
      $27 = $23 & 31;
      $28 = $27&255;
      $29 = 1 << $28;
      $30 = $26 & $29;
      $31 = ($30|0)==(0);
      if (!($31)) {
       $$019$sink = $$01922;
       break L1;
      }
      $32 = ((($$01922)) + 1|0);
      $33 = HEAP8[$32>>0]|0;
      $34 = ($33<<24>>24)==(0);
      if ($34) {
       $$019$sink = $32;
       break;
      } else {
       $$01922 = $32;$23 = $33;
      }
     }
    }
   }
  }
 } while(0);
 if ((label|0) == 3) {
  $8 = $3 << 24 >> 24;
  $9 = (___strchrnul($0,$8)|0);
  $$019$sink = $9;
 }
 $35 = $$019$sink;
 $36 = $0;
 $37 = (($35) - ($36))|0;
 STACKTOP = sp;return ($37|0);
}
function _strcat($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $2 = 0, $3 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = (_strlen($0)|0);
 $3 = (($0) + ($2)|0);
 (_strcpy($3,$1)|0);
 return ($0|0);
}
function _strtok($0,$1) {
 $0 = $0|0;
 $1 = $1|0;
 var $$0 = 0, $$010 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0, $8 = 0, $9 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $2 = ($0|0)==(0|0);
 if ($2) {
  $3 = HEAP32[4108]|0;
  $4 = ($3|0)==(0|0);
  if ($4) {
   $$0 = 0;
  } else {
   $$010 = $3;
   label = 3;
  }
 } else {
  $$010 = $0;
  label = 3;
 }
 do {
  if ((label|0) == 3) {
   $5 = (_strspn($$010,$1)|0);
   $6 = (($$010) + ($5)|0);
   $7 = HEAP8[$6>>0]|0;
   $8 = ($7<<24>>24)==(0);
   if ($8) {
    HEAP32[4108] = 0;
    $$0 = 0;
    break;
   }
   $9 = (_strcspn($6,$1)|0);
   $10 = (($6) + ($9)|0);
   HEAP32[4108] = $10;
   $11 = HEAP8[$10>>0]|0;
   $12 = ($11<<24>>24)==(0);
   if ($12) {
    HEAP32[4108] = 0;
    $$0 = $6;
    break;
   } else {
    $13 = ((($10)) + 1|0);
    HEAP32[4108] = $13;
    HEAP8[$10>>0] = 0;
    $$0 = $6;
    break;
   }
  }
 } while(0);
 return ($$0|0);
}
function _malloc($0) {
 $0 = $0|0;
 var $$$0192$i = 0, $$$0193$i = 0, $$$4236$i = 0, $$$4351$i = 0, $$$i = 0, $$0 = 0, $$0$i$i = 0, $$0$i$i$i = 0, $$0$i18$i = 0, $$01$i$i = 0, $$0189$i = 0, $$0192$lcssa$i = 0, $$01928$i = 0, $$0193$lcssa$i = 0, $$01937$i = 0, $$0197 = 0, $$0199 = 0, $$0206$i$i = 0, $$0207$i$i = 0, $$0211$i$i = 0;
 var $$0212$i$i = 0, $$024371$i = 0, $$0287$i$i = 0, $$0288$i$i = 0, $$0289$i$i = 0, $$0295$i$i = 0, $$0296$i$i = 0, $$0342$i = 0, $$0344$i = 0, $$0345$i = 0, $$0347$i = 0, $$0353$i = 0, $$0358$i = 0, $$0359$$i = 0, $$0359$i = 0, $$0361$i = 0, $$0362$i = 0, $$0368$i = 0, $$1196$i = 0, $$1198$i = 0;
 var $$124470$i = 0, $$1291$i$i = 0, $$1293$i$i = 0, $$1343$i = 0, $$1348$i = 0, $$1363$i = 0, $$1370$i = 0, $$1374$i = 0, $$2234253237$i = 0, $$2247$ph$i = 0, $$2253$ph$i = 0, $$2355$i = 0, $$3$i = 0, $$3$i$i = 0, $$3$i201 = 0, $$3350$i = 0, $$3372$i = 0, $$4$lcssa$i = 0, $$4$ph$i = 0, $$415$i = 0;
 var $$4236$i = 0, $$4351$lcssa$i = 0, $$435114$i = 0, $$4357$$4$i = 0, $$4357$ph$i = 0, $$435713$i = 0, $$723948$i = 0, $$749$i = 0, $$pre = 0, $$pre$i = 0, $$pre$i$i = 0, $$pre$i19$i = 0, $$pre$i210 = 0, $$pre$i212 = 0, $$pre$phi$i$iZ2D = 0, $$pre$phi$i20$iZ2D = 0, $$pre$phi$i211Z2D = 0, $$pre$phi$iZ2D = 0, $$pre$phi11$i$iZ2D = 0, $$pre$phiZ2D = 0;
 var $$pre10$i$i = 0, $$sink1$i = 0, $$sink1$i$i = 0, $$sink16$i = 0, $$sink2$i = 0, $$sink2$i204 = 0, $$sink3$i = 0, $1 = 0, $10 = 0, $100 = 0, $1000 = 0, $1001 = 0, $1002 = 0, $1003 = 0, $1004 = 0, $1005 = 0, $1006 = 0, $1007 = 0, $1008 = 0, $1009 = 0;
 var $101 = 0, $1010 = 0, $1011 = 0, $1012 = 0, $1013 = 0, $1014 = 0, $1015 = 0, $1016 = 0, $1017 = 0, $1018 = 0, $1019 = 0, $102 = 0, $1020 = 0, $1021 = 0, $1022 = 0, $1023 = 0, $1024 = 0, $1025 = 0, $1026 = 0, $1027 = 0;
 var $1028 = 0, $1029 = 0, $103 = 0, $1030 = 0, $1031 = 0, $1032 = 0, $1033 = 0, $1034 = 0, $1035 = 0, $1036 = 0, $1037 = 0, $1038 = 0, $1039 = 0, $104 = 0, $1040 = 0, $1041 = 0, $1042 = 0, $1043 = 0, $1044 = 0, $1045 = 0;
 var $1046 = 0, $1047 = 0, $1048 = 0, $1049 = 0, $105 = 0, $1050 = 0, $1051 = 0, $1052 = 0, $1053 = 0, $1054 = 0, $1055 = 0, $1056 = 0, $1057 = 0, $1058 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0;
 var $111 = 0, $112 = 0, $113 = 0, $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0;
 var $13 = 0, $130 = 0, $131 = 0, $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0;
 var $148 = 0, $149 = 0, $15 = 0, $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0, $16 = 0, $160 = 0, $161 = 0, $162 = 0, $163 = 0, $164 = 0, $165 = 0;
 var $166 = 0, $167 = 0, $168 = 0, $169 = 0, $17 = 0, $170 = 0, $171 = 0, $172 = 0, $173 = 0, $174 = 0, $175 = 0, $176 = 0, $177 = 0, $178 = 0, $179 = 0, $18 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0;
 var $184 = 0, $185 = 0, $186 = 0, $187 = 0, $188 = 0, $189 = 0, $19 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0, $197 = 0, $198 = 0, $199 = 0, $2 = 0, $20 = 0, $200 = 0;
 var $201 = 0, $202 = 0, $203 = 0, $204 = 0, $205 = 0, $206 = 0, $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0, $212 = 0, $213 = 0, $214 = 0, $215 = 0, $216 = 0, $217 = 0, $218 = 0, $219 = 0;
 var $22 = 0, $220 = 0, $221 = 0, $222 = 0, $223 = 0, $224 = 0, $225 = 0, $226 = 0, $227 = 0, $228 = 0, $229 = 0, $23 = 0, $230 = 0, $231 = 0, $232 = 0, $233 = 0, $234 = 0, $235 = 0, $236 = 0, $237 = 0;
 var $238 = 0, $239 = 0, $24 = 0, $240 = 0, $241 = 0, $242 = 0, $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0, $249 = 0, $25 = 0, $250 = 0, $251 = 0, $252 = 0, $253 = 0, $254 = 0, $255 = 0;
 var $256 = 0, $257 = 0, $258 = 0, $259 = 0, $26 = 0, $260 = 0, $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0, $267 = 0, $268 = 0, $269 = 0, $27 = 0, $270 = 0, $271 = 0, $272 = 0, $273 = 0;
 var $274 = 0, $275 = 0, $276 = 0, $277 = 0, $278 = 0, $279 = 0, $28 = 0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0, $285 = 0, $286 = 0, $287 = 0, $288 = 0, $289 = 0, $29 = 0, $290 = 0, $291 = 0;
 var $292 = 0, $293 = 0, $294 = 0, $295 = 0, $296 = 0, $297 = 0, $298 = 0, $299 = 0, $3 = 0, $30 = 0, $300 = 0, $301 = 0, $302 = 0, $303 = 0, $304 = 0, $305 = 0, $306 = 0, $307 = 0, $308 = 0, $309 = 0;
 var $31 = 0, $310 = 0, $311 = 0, $312 = 0, $313 = 0, $314 = 0, $315 = 0, $316 = 0, $317 = 0, $318 = 0, $319 = 0, $32 = 0, $320 = 0, $321 = 0, $322 = 0, $323 = 0, $324 = 0, $325 = 0, $326 = 0, $327 = 0;
 var $328 = 0, $329 = 0, $33 = 0, $330 = 0, $331 = 0, $332 = 0, $333 = 0, $334 = 0, $335 = 0, $336 = 0, $337 = 0, $338 = 0, $339 = 0, $34 = 0, $340 = 0, $341 = 0, $342 = 0, $343 = 0, $344 = 0, $345 = 0;
 var $346 = 0, $347 = 0, $348 = 0, $349 = 0, $35 = 0, $350 = 0, $351 = 0, $352 = 0, $353 = 0, $354 = 0, $355 = 0, $356 = 0, $357 = 0, $358 = 0, $359 = 0, $36 = 0, $360 = 0, $361 = 0, $362 = 0, $363 = 0;
 var $364 = 0, $365 = 0, $366 = 0, $367 = 0, $368 = 0, $369 = 0, $37 = 0, $370 = 0, $371 = 0, $372 = 0, $373 = 0, $374 = 0, $375 = 0, $376 = 0, $377 = 0, $378 = 0, $379 = 0, $38 = 0, $380 = 0, $381 = 0;
 var $382 = 0, $383 = 0, $384 = 0, $385 = 0, $386 = 0, $387 = 0, $388 = 0, $389 = 0, $39 = 0, $390 = 0, $391 = 0, $392 = 0, $393 = 0, $394 = 0, $395 = 0, $396 = 0, $397 = 0, $398 = 0, $399 = 0, $4 = 0;
 var $40 = 0, $400 = 0, $401 = 0, $402 = 0, $403 = 0, $404 = 0, $405 = 0, $406 = 0, $407 = 0, $408 = 0, $409 = 0, $41 = 0, $410 = 0, $411 = 0, $412 = 0, $413 = 0, $414 = 0, $415 = 0, $416 = 0, $417 = 0;
 var $418 = 0, $419 = 0, $42 = 0, $420 = 0, $421 = 0, $422 = 0, $423 = 0, $424 = 0, $425 = 0, $426 = 0, $427 = 0, $428 = 0, $429 = 0, $43 = 0, $430 = 0, $431 = 0, $432 = 0, $433 = 0, $434 = 0, $435 = 0;
 var $436 = 0, $437 = 0, $438 = 0, $439 = 0, $44 = 0, $440 = 0, $441 = 0, $442 = 0, $443 = 0, $444 = 0, $445 = 0, $446 = 0, $447 = 0, $448 = 0, $449 = 0, $45 = 0, $450 = 0, $451 = 0, $452 = 0, $453 = 0;
 var $454 = 0, $455 = 0, $456 = 0, $457 = 0, $458 = 0, $459 = 0, $46 = 0, $460 = 0, $461 = 0, $462 = 0, $463 = 0, $464 = 0, $465 = 0, $466 = 0, $467 = 0, $468 = 0, $469 = 0, $47 = 0, $470 = 0, $471 = 0;
 var $472 = 0, $473 = 0, $474 = 0, $475 = 0, $476 = 0, $477 = 0, $478 = 0, $479 = 0, $48 = 0, $480 = 0, $481 = 0, $482 = 0, $483 = 0, $484 = 0, $485 = 0, $486 = 0, $487 = 0, $488 = 0, $489 = 0, $49 = 0;
 var $490 = 0, $491 = 0, $492 = 0, $493 = 0, $494 = 0, $495 = 0, $496 = 0, $497 = 0, $498 = 0, $499 = 0, $5 = 0, $50 = 0, $500 = 0, $501 = 0, $502 = 0, $503 = 0, $504 = 0, $505 = 0, $506 = 0, $507 = 0;
 var $508 = 0, $509 = 0, $51 = 0, $510 = 0, $511 = 0, $512 = 0, $513 = 0, $514 = 0, $515 = 0, $516 = 0, $517 = 0, $518 = 0, $519 = 0, $52 = 0, $520 = 0, $521 = 0, $522 = 0, $523 = 0, $524 = 0, $525 = 0;
 var $526 = 0, $527 = 0, $528 = 0, $529 = 0, $53 = 0, $530 = 0, $531 = 0, $532 = 0, $533 = 0, $534 = 0, $535 = 0, $536 = 0, $537 = 0, $538 = 0, $539 = 0, $54 = 0, $540 = 0, $541 = 0, $542 = 0, $543 = 0;
 var $544 = 0, $545 = 0, $546 = 0, $547 = 0, $548 = 0, $549 = 0, $55 = 0, $550 = 0, $551 = 0, $552 = 0, $553 = 0, $554 = 0, $555 = 0, $556 = 0, $557 = 0, $558 = 0, $559 = 0, $56 = 0, $560 = 0, $561 = 0;
 var $562 = 0, $563 = 0, $564 = 0, $565 = 0, $566 = 0, $567 = 0, $568 = 0, $569 = 0, $57 = 0, $570 = 0, $571 = 0, $572 = 0, $573 = 0, $574 = 0, $575 = 0, $576 = 0, $577 = 0, $578 = 0, $579 = 0, $58 = 0;
 var $580 = 0, $581 = 0, $582 = 0, $583 = 0, $584 = 0, $585 = 0, $586 = 0, $587 = 0, $588 = 0, $589 = 0, $59 = 0, $590 = 0, $591 = 0, $592 = 0, $593 = 0, $594 = 0, $595 = 0, $596 = 0, $597 = 0, $598 = 0;
 var $599 = 0, $6 = 0, $60 = 0, $600 = 0, $601 = 0, $602 = 0, $603 = 0, $604 = 0, $605 = 0, $606 = 0, $607 = 0, $608 = 0, $609 = 0, $61 = 0, $610 = 0, $611 = 0, $612 = 0, $613 = 0, $614 = 0, $615 = 0;
 var $616 = 0, $617 = 0, $618 = 0, $619 = 0, $62 = 0, $620 = 0, $621 = 0, $622 = 0, $623 = 0, $624 = 0, $625 = 0, $626 = 0, $627 = 0, $628 = 0, $629 = 0, $63 = 0, $630 = 0, $631 = 0, $632 = 0, $633 = 0;
 var $634 = 0, $635 = 0, $636 = 0, $637 = 0, $638 = 0, $639 = 0, $64 = 0, $640 = 0, $641 = 0, $642 = 0, $643 = 0, $644 = 0, $645 = 0, $646 = 0, $647 = 0, $648 = 0, $649 = 0, $65 = 0, $650 = 0, $651 = 0;
 var $652 = 0, $653 = 0, $654 = 0, $655 = 0, $656 = 0, $657 = 0, $658 = 0, $659 = 0, $66 = 0, $660 = 0, $661 = 0, $662 = 0, $663 = 0, $664 = 0, $665 = 0, $666 = 0, $667 = 0, $668 = 0, $669 = 0, $67 = 0;
 var $670 = 0, $671 = 0, $672 = 0, $673 = 0, $674 = 0, $675 = 0, $676 = 0, $677 = 0, $678 = 0, $679 = 0, $68 = 0, $680 = 0, $681 = 0, $682 = 0, $683 = 0, $684 = 0, $685 = 0, $686 = 0, $687 = 0, $688 = 0;
 var $689 = 0, $69 = 0, $690 = 0, $691 = 0, $692 = 0, $693 = 0, $694 = 0, $695 = 0, $696 = 0, $697 = 0, $698 = 0, $699 = 0, $7 = 0, $70 = 0, $700 = 0, $701 = 0, $702 = 0, $703 = 0, $704 = 0, $705 = 0;
 var $706 = 0, $707 = 0, $708 = 0, $709 = 0, $71 = 0, $710 = 0, $711 = 0, $712 = 0, $713 = 0, $714 = 0, $715 = 0, $716 = 0, $717 = 0, $718 = 0, $719 = 0, $72 = 0, $720 = 0, $721 = 0, $722 = 0, $723 = 0;
 var $724 = 0, $725 = 0, $726 = 0, $727 = 0, $728 = 0, $729 = 0, $73 = 0, $730 = 0, $731 = 0, $732 = 0, $733 = 0, $734 = 0, $735 = 0, $736 = 0, $737 = 0, $738 = 0, $739 = 0, $74 = 0, $740 = 0, $741 = 0;
 var $742 = 0, $743 = 0, $744 = 0, $745 = 0, $746 = 0, $747 = 0, $748 = 0, $749 = 0, $75 = 0, $750 = 0, $751 = 0, $752 = 0, $753 = 0, $754 = 0, $755 = 0, $756 = 0, $757 = 0, $758 = 0, $759 = 0, $76 = 0;
 var $760 = 0, $761 = 0, $762 = 0, $763 = 0, $764 = 0, $765 = 0, $766 = 0, $767 = 0, $768 = 0, $769 = 0, $77 = 0, $770 = 0, $771 = 0, $772 = 0, $773 = 0, $774 = 0, $775 = 0, $776 = 0, $777 = 0, $778 = 0;
 var $779 = 0, $78 = 0, $780 = 0, $781 = 0, $782 = 0, $783 = 0, $784 = 0, $785 = 0, $786 = 0, $787 = 0, $788 = 0, $789 = 0, $79 = 0, $790 = 0, $791 = 0, $792 = 0, $793 = 0, $794 = 0, $795 = 0, $796 = 0;
 var $797 = 0, $798 = 0, $799 = 0, $8 = 0, $80 = 0, $800 = 0, $801 = 0, $802 = 0, $803 = 0, $804 = 0, $805 = 0, $806 = 0, $807 = 0, $808 = 0, $809 = 0, $81 = 0, $810 = 0, $811 = 0, $812 = 0, $813 = 0;
 var $814 = 0, $815 = 0, $816 = 0, $817 = 0, $818 = 0, $819 = 0, $82 = 0, $820 = 0, $821 = 0, $822 = 0, $823 = 0, $824 = 0, $825 = 0, $826 = 0, $827 = 0, $828 = 0, $829 = 0, $83 = 0, $830 = 0, $831 = 0;
 var $832 = 0, $833 = 0, $834 = 0, $835 = 0, $836 = 0, $837 = 0, $838 = 0, $839 = 0, $84 = 0, $840 = 0, $841 = 0, $842 = 0, $843 = 0, $844 = 0, $845 = 0, $846 = 0, $847 = 0, $848 = 0, $849 = 0, $85 = 0;
 var $850 = 0, $851 = 0, $852 = 0, $853 = 0, $854 = 0, $855 = 0, $856 = 0, $857 = 0, $858 = 0, $859 = 0, $86 = 0, $860 = 0, $861 = 0, $862 = 0, $863 = 0, $864 = 0, $865 = 0, $866 = 0, $867 = 0, $868 = 0;
 var $869 = 0, $87 = 0, $870 = 0, $871 = 0, $872 = 0, $873 = 0, $874 = 0, $875 = 0, $876 = 0, $877 = 0, $878 = 0, $879 = 0, $88 = 0, $880 = 0, $881 = 0, $882 = 0, $883 = 0, $884 = 0, $885 = 0, $886 = 0;
 var $887 = 0, $888 = 0, $889 = 0, $89 = 0, $890 = 0, $891 = 0, $892 = 0, $893 = 0, $894 = 0, $895 = 0, $896 = 0, $897 = 0, $898 = 0, $899 = 0, $9 = 0, $90 = 0, $900 = 0, $901 = 0, $902 = 0, $903 = 0;
 var $904 = 0, $905 = 0, $906 = 0, $907 = 0, $908 = 0, $909 = 0, $91 = 0, $910 = 0, $911 = 0, $912 = 0, $913 = 0, $914 = 0, $915 = 0, $916 = 0, $917 = 0, $918 = 0, $919 = 0, $92 = 0, $920 = 0, $921 = 0;
 var $922 = 0, $923 = 0, $924 = 0, $925 = 0, $926 = 0, $927 = 0, $928 = 0, $929 = 0, $93 = 0, $930 = 0, $931 = 0, $932 = 0, $933 = 0, $934 = 0, $935 = 0, $936 = 0, $937 = 0, $938 = 0, $939 = 0, $94 = 0;
 var $940 = 0, $941 = 0, $942 = 0, $943 = 0, $944 = 0, $945 = 0, $946 = 0, $947 = 0, $948 = 0, $949 = 0, $95 = 0, $950 = 0, $951 = 0, $952 = 0, $953 = 0, $954 = 0, $955 = 0, $956 = 0, $957 = 0, $958 = 0;
 var $959 = 0, $96 = 0, $960 = 0, $961 = 0, $962 = 0, $963 = 0, $964 = 0, $965 = 0, $966 = 0, $967 = 0, $968 = 0, $969 = 0, $97 = 0, $970 = 0, $971 = 0, $972 = 0, $973 = 0, $974 = 0, $975 = 0, $976 = 0;
 var $977 = 0, $978 = 0, $979 = 0, $98 = 0, $980 = 0, $981 = 0, $982 = 0, $983 = 0, $984 = 0, $985 = 0, $986 = 0, $987 = 0, $988 = 0, $989 = 0, $99 = 0, $990 = 0, $991 = 0, $992 = 0, $993 = 0, $994 = 0;
 var $995 = 0, $996 = 0, $997 = 0, $998 = 0, $999 = 0, $cond$i = 0, $cond$i$i = 0, $cond$i208 = 0, $exitcond$i$i = 0, $not$$i = 0, $not$$i$i = 0, $not$$i17$i = 0, $not$$i209 = 0, $not$$i216 = 0, $not$1$i = 0, $not$1$i203 = 0, $not$5$i = 0, $not$7$i$i = 0, $not$8$i = 0, $not$9$i = 0;
 var $or$cond$i = 0, $or$cond$i214 = 0, $or$cond1$i = 0, $or$cond10$i = 0, $or$cond11$i = 0, $or$cond11$not$i = 0, $or$cond12$i = 0, $or$cond2$i = 0, $or$cond2$i215 = 0, $or$cond5$i = 0, $or$cond50$i = 0, $or$cond51$i = 0, $or$cond7$i = 0, label = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16|0; if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(16|0);
 $1 = sp;
 $2 = ($0>>>0)<(245);
 do {
  if ($2) {
   $3 = ($0>>>0)<(11);
   $4 = (($0) + 11)|0;
   $5 = $4 & -8;
   $6 = $3 ? 16 : $5;
   $7 = $6 >>> 3;
   $8 = HEAP32[4109]|0;
   $9 = $8 >>> $7;
   $10 = $9 & 3;
   $11 = ($10|0)==(0);
   if (!($11)) {
    $12 = $9 & 1;
    $13 = $12 ^ 1;
    $14 = (($13) + ($7))|0;
    $15 = $14 << 1;
    $16 = (16476 + ($15<<2)|0);
    $17 = ((($16)) + 8|0);
    $18 = HEAP32[$17>>2]|0;
    $19 = ((($18)) + 8|0);
    $20 = HEAP32[$19>>2]|0;
    $21 = ($16|0)==($20|0);
    do {
     if ($21) {
      $22 = 1 << $14;
      $23 = $22 ^ -1;
      $24 = $8 & $23;
      HEAP32[4109] = $24;
     } else {
      $25 = HEAP32[(16452)>>2]|0;
      $26 = ($20>>>0)<($25>>>0);
      if ($26) {
       _abort();
       // unreachable;
      }
      $27 = ((($20)) + 12|0);
      $28 = HEAP32[$27>>2]|0;
      $29 = ($28|0)==($18|0);
      if ($29) {
       HEAP32[$27>>2] = $16;
       HEAP32[$17>>2] = $20;
       break;
      } else {
       _abort();
       // unreachable;
      }
     }
    } while(0);
    $30 = $14 << 3;
    $31 = $30 | 3;
    $32 = ((($18)) + 4|0);
    HEAP32[$32>>2] = $31;
    $33 = (($18) + ($30)|0);
    $34 = ((($33)) + 4|0);
    $35 = HEAP32[$34>>2]|0;
    $36 = $35 | 1;
    HEAP32[$34>>2] = $36;
    $$0 = $19;
    STACKTOP = sp;return ($$0|0);
   }
   $37 = HEAP32[(16444)>>2]|0;
   $38 = ($6>>>0)>($37>>>0);
   if ($38) {
    $39 = ($9|0)==(0);
    if (!($39)) {
     $40 = $9 << $7;
     $41 = 2 << $7;
     $42 = (0 - ($41))|0;
     $43 = $41 | $42;
     $44 = $40 & $43;
     $45 = (0 - ($44))|0;
     $46 = $44 & $45;
     $47 = (($46) + -1)|0;
     $48 = $47 >>> 12;
     $49 = $48 & 16;
     $50 = $47 >>> $49;
     $51 = $50 >>> 5;
     $52 = $51 & 8;
     $53 = $52 | $49;
     $54 = $50 >>> $52;
     $55 = $54 >>> 2;
     $56 = $55 & 4;
     $57 = $53 | $56;
     $58 = $54 >>> $56;
     $59 = $58 >>> 1;
     $60 = $59 & 2;
     $61 = $57 | $60;
     $62 = $58 >>> $60;
     $63 = $62 >>> 1;
     $64 = $63 & 1;
     $65 = $61 | $64;
     $66 = $62 >>> $64;
     $67 = (($65) + ($66))|0;
     $68 = $67 << 1;
     $69 = (16476 + ($68<<2)|0);
     $70 = ((($69)) + 8|0);
     $71 = HEAP32[$70>>2]|0;
     $72 = ((($71)) + 8|0);
     $73 = HEAP32[$72>>2]|0;
     $74 = ($69|0)==($73|0);
     do {
      if ($74) {
       $75 = 1 << $67;
       $76 = $75 ^ -1;
       $77 = $8 & $76;
       HEAP32[4109] = $77;
       $98 = $77;
      } else {
       $78 = HEAP32[(16452)>>2]|0;
       $79 = ($73>>>0)<($78>>>0);
       if ($79) {
        _abort();
        // unreachable;
       }
       $80 = ((($73)) + 12|0);
       $81 = HEAP32[$80>>2]|0;
       $82 = ($81|0)==($71|0);
       if ($82) {
        HEAP32[$80>>2] = $69;
        HEAP32[$70>>2] = $73;
        $98 = $8;
        break;
       } else {
        _abort();
        // unreachable;
       }
      }
     } while(0);
     $83 = $67 << 3;
     $84 = (($83) - ($6))|0;
     $85 = $6 | 3;
     $86 = ((($71)) + 4|0);
     HEAP32[$86>>2] = $85;
     $87 = (($71) + ($6)|0);
     $88 = $84 | 1;
     $89 = ((($87)) + 4|0);
     HEAP32[$89>>2] = $88;
     $90 = (($87) + ($84)|0);
     HEAP32[$90>>2] = $84;
     $91 = ($37|0)==(0);
     if (!($91)) {
      $92 = HEAP32[(16456)>>2]|0;
      $93 = $37 >>> 3;
      $94 = $93 << 1;
      $95 = (16476 + ($94<<2)|0);
      $96 = 1 << $93;
      $97 = $98 & $96;
      $99 = ($97|0)==(0);
      if ($99) {
       $100 = $98 | $96;
       HEAP32[4109] = $100;
       $$pre = ((($95)) + 8|0);
       $$0199 = $95;$$pre$phiZ2D = $$pre;
      } else {
       $101 = ((($95)) + 8|0);
       $102 = HEAP32[$101>>2]|0;
       $103 = HEAP32[(16452)>>2]|0;
       $104 = ($102>>>0)<($103>>>0);
       if ($104) {
        _abort();
        // unreachable;
       } else {
        $$0199 = $102;$$pre$phiZ2D = $101;
       }
      }
      HEAP32[$$pre$phiZ2D>>2] = $92;
      $105 = ((($$0199)) + 12|0);
      HEAP32[$105>>2] = $92;
      $106 = ((($92)) + 8|0);
      HEAP32[$106>>2] = $$0199;
      $107 = ((($92)) + 12|0);
      HEAP32[$107>>2] = $95;
     }
     HEAP32[(16444)>>2] = $84;
     HEAP32[(16456)>>2] = $87;
     $$0 = $72;
     STACKTOP = sp;return ($$0|0);
    }
    $108 = HEAP32[(16440)>>2]|0;
    $109 = ($108|0)==(0);
    if ($109) {
     $$0197 = $6;
    } else {
     $110 = (0 - ($108))|0;
     $111 = $108 & $110;
     $112 = (($111) + -1)|0;
     $113 = $112 >>> 12;
     $114 = $113 & 16;
     $115 = $112 >>> $114;
     $116 = $115 >>> 5;
     $117 = $116 & 8;
     $118 = $117 | $114;
     $119 = $115 >>> $117;
     $120 = $119 >>> 2;
     $121 = $120 & 4;
     $122 = $118 | $121;
     $123 = $119 >>> $121;
     $124 = $123 >>> 1;
     $125 = $124 & 2;
     $126 = $122 | $125;
     $127 = $123 >>> $125;
     $128 = $127 >>> 1;
     $129 = $128 & 1;
     $130 = $126 | $129;
     $131 = $127 >>> $129;
     $132 = (($130) + ($131))|0;
     $133 = (16740 + ($132<<2)|0);
     $134 = HEAP32[$133>>2]|0;
     $135 = ((($134)) + 4|0);
     $136 = HEAP32[$135>>2]|0;
     $137 = $136 & -8;
     $138 = (($137) - ($6))|0;
     $139 = ((($134)) + 16|0);
     $140 = HEAP32[$139>>2]|0;
     $not$5$i = ($140|0)==(0|0);
     $$sink16$i = $not$5$i&1;
     $141 = (((($134)) + 16|0) + ($$sink16$i<<2)|0);
     $142 = HEAP32[$141>>2]|0;
     $143 = ($142|0)==(0|0);
     if ($143) {
      $$0192$lcssa$i = $134;$$0193$lcssa$i = $138;
     } else {
      $$01928$i = $134;$$01937$i = $138;$145 = $142;
      while(1) {
       $144 = ((($145)) + 4|0);
       $146 = HEAP32[$144>>2]|0;
       $147 = $146 & -8;
       $148 = (($147) - ($6))|0;
       $149 = ($148>>>0)<($$01937$i>>>0);
       $$$0193$i = $149 ? $148 : $$01937$i;
       $$$0192$i = $149 ? $145 : $$01928$i;
       $150 = ((($145)) + 16|0);
       $151 = HEAP32[$150>>2]|0;
       $not$$i = ($151|0)==(0|0);
       $$sink1$i = $not$$i&1;
       $152 = (((($145)) + 16|0) + ($$sink1$i<<2)|0);
       $153 = HEAP32[$152>>2]|0;
       $154 = ($153|0)==(0|0);
       if ($154) {
        $$0192$lcssa$i = $$$0192$i;$$0193$lcssa$i = $$$0193$i;
        break;
       } else {
        $$01928$i = $$$0192$i;$$01937$i = $$$0193$i;$145 = $153;
       }
      }
     }
     $155 = HEAP32[(16452)>>2]|0;
     $156 = ($$0192$lcssa$i>>>0)<($155>>>0);
     if ($156) {
      _abort();
      // unreachable;
     }
     $157 = (($$0192$lcssa$i) + ($6)|0);
     $158 = ($$0192$lcssa$i>>>0)<($157>>>0);
     if (!($158)) {
      _abort();
      // unreachable;
     }
     $159 = ((($$0192$lcssa$i)) + 24|0);
     $160 = HEAP32[$159>>2]|0;
     $161 = ((($$0192$lcssa$i)) + 12|0);
     $162 = HEAP32[$161>>2]|0;
     $163 = ($162|0)==($$0192$lcssa$i|0);
     do {
      if ($163) {
       $173 = ((($$0192$lcssa$i)) + 20|0);
       $174 = HEAP32[$173>>2]|0;
       $175 = ($174|0)==(0|0);
       if ($175) {
        $176 = ((($$0192$lcssa$i)) + 16|0);
        $177 = HEAP32[$176>>2]|0;
        $178 = ($177|0)==(0|0);
        if ($178) {
         $$3$i = 0;
         break;
        } else {
         $$1196$i = $177;$$1198$i = $176;
        }
       } else {
        $$1196$i = $174;$$1198$i = $173;
       }
       while(1) {
        $179 = ((($$1196$i)) + 20|0);
        $180 = HEAP32[$179>>2]|0;
        $181 = ($180|0)==(0|0);
        if (!($181)) {
         $$1196$i = $180;$$1198$i = $179;
         continue;
        }
        $182 = ((($$1196$i)) + 16|0);
        $183 = HEAP32[$182>>2]|0;
        $184 = ($183|0)==(0|0);
        if ($184) {
         break;
        } else {
         $$1196$i = $183;$$1198$i = $182;
        }
       }
       $185 = ($$1198$i>>>0)<($155>>>0);
       if ($185) {
        _abort();
        // unreachable;
       } else {
        HEAP32[$$1198$i>>2] = 0;
        $$3$i = $$1196$i;
        break;
       }
      } else {
       $164 = ((($$0192$lcssa$i)) + 8|0);
       $165 = HEAP32[$164>>2]|0;
       $166 = ($165>>>0)<($155>>>0);
       if ($166) {
        _abort();
        // unreachable;
       }
       $167 = ((($165)) + 12|0);
       $168 = HEAP32[$167>>2]|0;
       $169 = ($168|0)==($$0192$lcssa$i|0);
       if (!($169)) {
        _abort();
        // unreachable;
       }
       $170 = ((($162)) + 8|0);
       $171 = HEAP32[$170>>2]|0;
       $172 = ($171|0)==($$0192$lcssa$i|0);
       if ($172) {
        HEAP32[$167>>2] = $162;
        HEAP32[$170>>2] = $165;
        $$3$i = $162;
        break;
       } else {
        _abort();
        // unreachable;
       }
      }
     } while(0);
     $186 = ($160|0)==(0|0);
     L73: do {
      if (!($186)) {
       $187 = ((($$0192$lcssa$i)) + 28|0);
       $188 = HEAP32[$187>>2]|0;
       $189 = (16740 + ($188<<2)|0);
       $190 = HEAP32[$189>>2]|0;
       $191 = ($$0192$lcssa$i|0)==($190|0);
       do {
        if ($191) {
         HEAP32[$189>>2] = $$3$i;
         $cond$i = ($$3$i|0)==(0|0);
         if ($cond$i) {
          $192 = 1 << $188;
          $193 = $192 ^ -1;
          $194 = $108 & $193;
          HEAP32[(16440)>>2] = $194;
          break L73;
         }
        } else {
         $195 = HEAP32[(16452)>>2]|0;
         $196 = ($160>>>0)<($195>>>0);
         if ($196) {
          _abort();
          // unreachable;
         } else {
          $197 = ((($160)) + 16|0);
          $198 = HEAP32[$197>>2]|0;
          $not$1$i = ($198|0)!=($$0192$lcssa$i|0);
          $$sink2$i = $not$1$i&1;
          $199 = (((($160)) + 16|0) + ($$sink2$i<<2)|0);
          HEAP32[$199>>2] = $$3$i;
          $200 = ($$3$i|0)==(0|0);
          if ($200) {
           break L73;
          } else {
           break;
          }
         }
        }
       } while(0);
       $201 = HEAP32[(16452)>>2]|0;
       $202 = ($$3$i>>>0)<($201>>>0);
       if ($202) {
        _abort();
        // unreachable;
       }
       $203 = ((($$3$i)) + 24|0);
       HEAP32[$203>>2] = $160;
       $204 = ((($$0192$lcssa$i)) + 16|0);
       $205 = HEAP32[$204>>2]|0;
       $206 = ($205|0)==(0|0);
       do {
        if (!($206)) {
         $207 = ($205>>>0)<($201>>>0);
         if ($207) {
          _abort();
          // unreachable;
         } else {
          $208 = ((($$3$i)) + 16|0);
          HEAP32[$208>>2] = $205;
          $209 = ((($205)) + 24|0);
          HEAP32[$209>>2] = $$3$i;
          break;
         }
        }
       } while(0);
       $210 = ((($$0192$lcssa$i)) + 20|0);
       $211 = HEAP32[$210>>2]|0;
       $212 = ($211|0)==(0|0);
       if (!($212)) {
        $213 = HEAP32[(16452)>>2]|0;
        $214 = ($211>>>0)<($213>>>0);
        if ($214) {
         _abort();
         // unreachable;
        } else {
         $215 = ((($$3$i)) + 20|0);
         HEAP32[$215>>2] = $211;
         $216 = ((($211)) + 24|0);
         HEAP32[$216>>2] = $$3$i;
         break;
        }
       }
      }
     } while(0);
     $217 = ($$0193$lcssa$i>>>0)<(16);
     if ($217) {
      $218 = (($$0193$lcssa$i) + ($6))|0;
      $219 = $218 | 3;
      $220 = ((($$0192$lcssa$i)) + 4|0);
      HEAP32[$220>>2] = $219;
      $221 = (($$0192$lcssa$i) + ($218)|0);
      $222 = ((($221)) + 4|0);
      $223 = HEAP32[$222>>2]|0;
      $224 = $223 | 1;
      HEAP32[$222>>2] = $224;
     } else {
      $225 = $6 | 3;
      $226 = ((($$0192$lcssa$i)) + 4|0);
      HEAP32[$226>>2] = $225;
      $227 = $$0193$lcssa$i | 1;
      $228 = ((($157)) + 4|0);
      HEAP32[$228>>2] = $227;
      $229 = (($157) + ($$0193$lcssa$i)|0);
      HEAP32[$229>>2] = $$0193$lcssa$i;
      $230 = ($37|0)==(0);
      if (!($230)) {
       $231 = HEAP32[(16456)>>2]|0;
       $232 = $37 >>> 3;
       $233 = $232 << 1;
       $234 = (16476 + ($233<<2)|0);
       $235 = 1 << $232;
       $236 = $8 & $235;
       $237 = ($236|0)==(0);
       if ($237) {
        $238 = $8 | $235;
        HEAP32[4109] = $238;
        $$pre$i = ((($234)) + 8|0);
        $$0189$i = $234;$$pre$phi$iZ2D = $$pre$i;
       } else {
        $239 = ((($234)) + 8|0);
        $240 = HEAP32[$239>>2]|0;
        $241 = HEAP32[(16452)>>2]|0;
        $242 = ($240>>>0)<($241>>>0);
        if ($242) {
         _abort();
         // unreachable;
        } else {
         $$0189$i = $240;$$pre$phi$iZ2D = $239;
        }
       }
       HEAP32[$$pre$phi$iZ2D>>2] = $231;
       $243 = ((($$0189$i)) + 12|0);
       HEAP32[$243>>2] = $231;
       $244 = ((($231)) + 8|0);
       HEAP32[$244>>2] = $$0189$i;
       $245 = ((($231)) + 12|0);
       HEAP32[$245>>2] = $234;
      }
      HEAP32[(16444)>>2] = $$0193$lcssa$i;
      HEAP32[(16456)>>2] = $157;
     }
     $246 = ((($$0192$lcssa$i)) + 8|0);
     $$0 = $246;
     STACKTOP = sp;return ($$0|0);
    }
   } else {
    $$0197 = $6;
   }
  } else {
   $247 = ($0>>>0)>(4294967231);
   if ($247) {
    $$0197 = -1;
   } else {
    $248 = (($0) + 11)|0;
    $249 = $248 & -8;
    $250 = HEAP32[(16440)>>2]|0;
    $251 = ($250|0)==(0);
    if ($251) {
     $$0197 = $249;
    } else {
     $252 = (0 - ($249))|0;
     $253 = $248 >>> 8;
     $254 = ($253|0)==(0);
     if ($254) {
      $$0358$i = 0;
     } else {
      $255 = ($249>>>0)>(16777215);
      if ($255) {
       $$0358$i = 31;
      } else {
       $256 = (($253) + 1048320)|0;
       $257 = $256 >>> 16;
       $258 = $257 & 8;
       $259 = $253 << $258;
       $260 = (($259) + 520192)|0;
       $261 = $260 >>> 16;
       $262 = $261 & 4;
       $263 = $262 | $258;
       $264 = $259 << $262;
       $265 = (($264) + 245760)|0;
       $266 = $265 >>> 16;
       $267 = $266 & 2;
       $268 = $263 | $267;
       $269 = (14 - ($268))|0;
       $270 = $264 << $267;
       $271 = $270 >>> 15;
       $272 = (($269) + ($271))|0;
       $273 = $272 << 1;
       $274 = (($272) + 7)|0;
       $275 = $249 >>> $274;
       $276 = $275 & 1;
       $277 = $276 | $273;
       $$0358$i = $277;
      }
     }
     $278 = (16740 + ($$0358$i<<2)|0);
     $279 = HEAP32[$278>>2]|0;
     $280 = ($279|0)==(0|0);
     L117: do {
      if ($280) {
       $$2355$i = 0;$$3$i201 = 0;$$3350$i = $252;
       label = 81;
      } else {
       $281 = ($$0358$i|0)==(31);
       $282 = $$0358$i >>> 1;
       $283 = (25 - ($282))|0;
       $284 = $281 ? 0 : $283;
       $285 = $249 << $284;
       $$0342$i = 0;$$0347$i = $252;$$0353$i = $279;$$0359$i = $285;$$0362$i = 0;
       while(1) {
        $286 = ((($$0353$i)) + 4|0);
        $287 = HEAP32[$286>>2]|0;
        $288 = $287 & -8;
        $289 = (($288) - ($249))|0;
        $290 = ($289>>>0)<($$0347$i>>>0);
        if ($290) {
         $291 = ($289|0)==(0);
         if ($291) {
          $$415$i = $$0353$i;$$435114$i = 0;$$435713$i = $$0353$i;
          label = 85;
          break L117;
         } else {
          $$1343$i = $$0353$i;$$1348$i = $289;
         }
        } else {
         $$1343$i = $$0342$i;$$1348$i = $$0347$i;
        }
        $292 = ((($$0353$i)) + 20|0);
        $293 = HEAP32[$292>>2]|0;
        $294 = $$0359$i >>> 31;
        $295 = (((($$0353$i)) + 16|0) + ($294<<2)|0);
        $296 = HEAP32[$295>>2]|0;
        $297 = ($293|0)==(0|0);
        $298 = ($293|0)==($296|0);
        $or$cond2$i = $297 | $298;
        $$1363$i = $or$cond2$i ? $$0362$i : $293;
        $299 = ($296|0)==(0|0);
        $not$8$i = $299 ^ 1;
        $300 = $not$8$i&1;
        $$0359$$i = $$0359$i << $300;
        if ($299) {
         $$2355$i = $$1363$i;$$3$i201 = $$1343$i;$$3350$i = $$1348$i;
         label = 81;
         break;
        } else {
         $$0342$i = $$1343$i;$$0347$i = $$1348$i;$$0353$i = $296;$$0359$i = $$0359$$i;$$0362$i = $$1363$i;
        }
       }
      }
     } while(0);
     if ((label|0) == 81) {
      $301 = ($$2355$i|0)==(0|0);
      $302 = ($$3$i201|0)==(0|0);
      $or$cond$i = $301 & $302;
      if ($or$cond$i) {
       $303 = 2 << $$0358$i;
       $304 = (0 - ($303))|0;
       $305 = $303 | $304;
       $306 = $250 & $305;
       $307 = ($306|0)==(0);
       if ($307) {
        $$0197 = $249;
        break;
       }
       $308 = (0 - ($306))|0;
       $309 = $306 & $308;
       $310 = (($309) + -1)|0;
       $311 = $310 >>> 12;
       $312 = $311 & 16;
       $313 = $310 >>> $312;
       $314 = $313 >>> 5;
       $315 = $314 & 8;
       $316 = $315 | $312;
       $317 = $313 >>> $315;
       $318 = $317 >>> 2;
       $319 = $318 & 4;
       $320 = $316 | $319;
       $321 = $317 >>> $319;
       $322 = $321 >>> 1;
       $323 = $322 & 2;
       $324 = $320 | $323;
       $325 = $321 >>> $323;
       $326 = $325 >>> 1;
       $327 = $326 & 1;
       $328 = $324 | $327;
       $329 = $325 >>> $327;
       $330 = (($328) + ($329))|0;
       $331 = (16740 + ($330<<2)|0);
       $332 = HEAP32[$331>>2]|0;
       $$4$ph$i = 0;$$4357$ph$i = $332;
      } else {
       $$4$ph$i = $$3$i201;$$4357$ph$i = $$2355$i;
      }
      $333 = ($$4357$ph$i|0)==(0|0);
      if ($333) {
       $$4$lcssa$i = $$4$ph$i;$$4351$lcssa$i = $$3350$i;
      } else {
       $$415$i = $$4$ph$i;$$435114$i = $$3350$i;$$435713$i = $$4357$ph$i;
       label = 85;
      }
     }
     if ((label|0) == 85) {
      while(1) {
       label = 0;
       $334 = ((($$435713$i)) + 4|0);
       $335 = HEAP32[$334>>2]|0;
       $336 = $335 & -8;
       $337 = (($336) - ($249))|0;
       $338 = ($337>>>0)<($$435114$i>>>0);
       $$$4351$i = $338 ? $337 : $$435114$i;
       $$4357$$4$i = $338 ? $$435713$i : $$415$i;
       $339 = ((($$435713$i)) + 16|0);
       $340 = HEAP32[$339>>2]|0;
       $not$1$i203 = ($340|0)==(0|0);
       $$sink2$i204 = $not$1$i203&1;
       $341 = (((($$435713$i)) + 16|0) + ($$sink2$i204<<2)|0);
       $342 = HEAP32[$341>>2]|0;
       $343 = ($342|0)==(0|0);
       if ($343) {
        $$4$lcssa$i = $$4357$$4$i;$$4351$lcssa$i = $$$4351$i;
        break;
       } else {
        $$415$i = $$4357$$4$i;$$435114$i = $$$4351$i;$$435713$i = $342;
        label = 85;
       }
      }
     }
     $344 = ($$4$lcssa$i|0)==(0|0);
     if ($344) {
      $$0197 = $249;
     } else {
      $345 = HEAP32[(16444)>>2]|0;
      $346 = (($345) - ($249))|0;
      $347 = ($$4351$lcssa$i>>>0)<($346>>>0);
      if ($347) {
       $348 = HEAP32[(16452)>>2]|0;
       $349 = ($$4$lcssa$i>>>0)<($348>>>0);
       if ($349) {
        _abort();
        // unreachable;
       }
       $350 = (($$4$lcssa$i) + ($249)|0);
       $351 = ($$4$lcssa$i>>>0)<($350>>>0);
       if (!($351)) {
        _abort();
        // unreachable;
       }
       $352 = ((($$4$lcssa$i)) + 24|0);
       $353 = HEAP32[$352>>2]|0;
       $354 = ((($$4$lcssa$i)) + 12|0);
       $355 = HEAP32[$354>>2]|0;
       $356 = ($355|0)==($$4$lcssa$i|0);
       do {
        if ($356) {
         $366 = ((($$4$lcssa$i)) + 20|0);
         $367 = HEAP32[$366>>2]|0;
         $368 = ($367|0)==(0|0);
         if ($368) {
          $369 = ((($$4$lcssa$i)) + 16|0);
          $370 = HEAP32[$369>>2]|0;
          $371 = ($370|0)==(0|0);
          if ($371) {
           $$3372$i = 0;
           break;
          } else {
           $$1370$i = $370;$$1374$i = $369;
          }
         } else {
          $$1370$i = $367;$$1374$i = $366;
         }
         while(1) {
          $372 = ((($$1370$i)) + 20|0);
          $373 = HEAP32[$372>>2]|0;
          $374 = ($373|0)==(0|0);
          if (!($374)) {
           $$1370$i = $373;$$1374$i = $372;
           continue;
          }
          $375 = ((($$1370$i)) + 16|0);
          $376 = HEAP32[$375>>2]|0;
          $377 = ($376|0)==(0|0);
          if ($377) {
           break;
          } else {
           $$1370$i = $376;$$1374$i = $375;
          }
         }
         $378 = ($$1374$i>>>0)<($348>>>0);
         if ($378) {
          _abort();
          // unreachable;
         } else {
          HEAP32[$$1374$i>>2] = 0;
          $$3372$i = $$1370$i;
          break;
         }
        } else {
         $357 = ((($$4$lcssa$i)) + 8|0);
         $358 = HEAP32[$357>>2]|0;
         $359 = ($358>>>0)<($348>>>0);
         if ($359) {
          _abort();
          // unreachable;
         }
         $360 = ((($358)) + 12|0);
         $361 = HEAP32[$360>>2]|0;
         $362 = ($361|0)==($$4$lcssa$i|0);
         if (!($362)) {
          _abort();
          // unreachable;
         }
         $363 = ((($355)) + 8|0);
         $364 = HEAP32[$363>>2]|0;
         $365 = ($364|0)==($$4$lcssa$i|0);
         if ($365) {
          HEAP32[$360>>2] = $355;
          HEAP32[$363>>2] = $358;
          $$3372$i = $355;
          break;
         } else {
          _abort();
          // unreachable;
         }
        }
       } while(0);
       $379 = ($353|0)==(0|0);
       L164: do {
        if ($379) {
         $470 = $250;
        } else {
         $380 = ((($$4$lcssa$i)) + 28|0);
         $381 = HEAP32[$380>>2]|0;
         $382 = (16740 + ($381<<2)|0);
         $383 = HEAP32[$382>>2]|0;
         $384 = ($$4$lcssa$i|0)==($383|0);
         do {
          if ($384) {
           HEAP32[$382>>2] = $$3372$i;
           $cond$i208 = ($$3372$i|0)==(0|0);
           if ($cond$i208) {
            $385 = 1 << $381;
            $386 = $385 ^ -1;
            $387 = $250 & $386;
            HEAP32[(16440)>>2] = $387;
            $470 = $387;
            break L164;
           }
          } else {
           $388 = HEAP32[(16452)>>2]|0;
           $389 = ($353>>>0)<($388>>>0);
           if ($389) {
            _abort();
            // unreachable;
           } else {
            $390 = ((($353)) + 16|0);
            $391 = HEAP32[$390>>2]|0;
            $not$$i209 = ($391|0)!=($$4$lcssa$i|0);
            $$sink3$i = $not$$i209&1;
            $392 = (((($353)) + 16|0) + ($$sink3$i<<2)|0);
            HEAP32[$392>>2] = $$3372$i;
            $393 = ($$3372$i|0)==(0|0);
            if ($393) {
             $470 = $250;
             break L164;
            } else {
             break;
            }
           }
          }
         } while(0);
         $394 = HEAP32[(16452)>>2]|0;
         $395 = ($$3372$i>>>0)<($394>>>0);
         if ($395) {
          _abort();
          // unreachable;
         }
         $396 = ((($$3372$i)) + 24|0);
         HEAP32[$396>>2] = $353;
         $397 = ((($$4$lcssa$i)) + 16|0);
         $398 = HEAP32[$397>>2]|0;
         $399 = ($398|0)==(0|0);
         do {
          if (!($399)) {
           $400 = ($398>>>0)<($394>>>0);
           if ($400) {
            _abort();
            // unreachable;
           } else {
            $401 = ((($$3372$i)) + 16|0);
            HEAP32[$401>>2] = $398;
            $402 = ((($398)) + 24|0);
            HEAP32[$402>>2] = $$3372$i;
            break;
           }
          }
         } while(0);
         $403 = ((($$4$lcssa$i)) + 20|0);
         $404 = HEAP32[$403>>2]|0;
         $405 = ($404|0)==(0|0);
         if ($405) {
          $470 = $250;
         } else {
          $406 = HEAP32[(16452)>>2]|0;
          $407 = ($404>>>0)<($406>>>0);
          if ($407) {
           _abort();
           // unreachable;
          } else {
           $408 = ((($$3372$i)) + 20|0);
           HEAP32[$408>>2] = $404;
           $409 = ((($404)) + 24|0);
           HEAP32[$409>>2] = $$3372$i;
           $470 = $250;
           break;
          }
         }
        }
       } while(0);
       $410 = ($$4351$lcssa$i>>>0)<(16);
       do {
        if ($410) {
         $411 = (($$4351$lcssa$i) + ($249))|0;
         $412 = $411 | 3;
         $413 = ((($$4$lcssa$i)) + 4|0);
         HEAP32[$413>>2] = $412;
         $414 = (($$4$lcssa$i) + ($411)|0);
         $415 = ((($414)) + 4|0);
         $416 = HEAP32[$415>>2]|0;
         $417 = $416 | 1;
         HEAP32[$415>>2] = $417;
        } else {
         $418 = $249 | 3;
         $419 = ((($$4$lcssa$i)) + 4|0);
         HEAP32[$419>>2] = $418;
         $420 = $$4351$lcssa$i | 1;
         $421 = ((($350)) + 4|0);
         HEAP32[$421>>2] = $420;
         $422 = (($350) + ($$4351$lcssa$i)|0);
         HEAP32[$422>>2] = $$4351$lcssa$i;
         $423 = $$4351$lcssa$i >>> 3;
         $424 = ($$4351$lcssa$i>>>0)<(256);
         if ($424) {
          $425 = $423 << 1;
          $426 = (16476 + ($425<<2)|0);
          $427 = HEAP32[4109]|0;
          $428 = 1 << $423;
          $429 = $427 & $428;
          $430 = ($429|0)==(0);
          if ($430) {
           $431 = $427 | $428;
           HEAP32[4109] = $431;
           $$pre$i210 = ((($426)) + 8|0);
           $$0368$i = $426;$$pre$phi$i211Z2D = $$pre$i210;
          } else {
           $432 = ((($426)) + 8|0);
           $433 = HEAP32[$432>>2]|0;
           $434 = HEAP32[(16452)>>2]|0;
           $435 = ($433>>>0)<($434>>>0);
           if ($435) {
            _abort();
            // unreachable;
           } else {
            $$0368$i = $433;$$pre$phi$i211Z2D = $432;
           }
          }
          HEAP32[$$pre$phi$i211Z2D>>2] = $350;
          $436 = ((($$0368$i)) + 12|0);
          HEAP32[$436>>2] = $350;
          $437 = ((($350)) + 8|0);
          HEAP32[$437>>2] = $$0368$i;
          $438 = ((($350)) + 12|0);
          HEAP32[$438>>2] = $426;
          break;
         }
         $439 = $$4351$lcssa$i >>> 8;
         $440 = ($439|0)==(0);
         if ($440) {
          $$0361$i = 0;
         } else {
          $441 = ($$4351$lcssa$i>>>0)>(16777215);
          if ($441) {
           $$0361$i = 31;
          } else {
           $442 = (($439) + 1048320)|0;
           $443 = $442 >>> 16;
           $444 = $443 & 8;
           $445 = $439 << $444;
           $446 = (($445) + 520192)|0;
           $447 = $446 >>> 16;
           $448 = $447 & 4;
           $449 = $448 | $444;
           $450 = $445 << $448;
           $451 = (($450) + 245760)|0;
           $452 = $451 >>> 16;
           $453 = $452 & 2;
           $454 = $449 | $453;
           $455 = (14 - ($454))|0;
           $456 = $450 << $453;
           $457 = $456 >>> 15;
           $458 = (($455) + ($457))|0;
           $459 = $458 << 1;
           $460 = (($458) + 7)|0;
           $461 = $$4351$lcssa$i >>> $460;
           $462 = $461 & 1;
           $463 = $462 | $459;
           $$0361$i = $463;
          }
         }
         $464 = (16740 + ($$0361$i<<2)|0);
         $465 = ((($350)) + 28|0);
         HEAP32[$465>>2] = $$0361$i;
         $466 = ((($350)) + 16|0);
         $467 = ((($466)) + 4|0);
         HEAP32[$467>>2] = 0;
         HEAP32[$466>>2] = 0;
         $468 = 1 << $$0361$i;
         $469 = $470 & $468;
         $471 = ($469|0)==(0);
         if ($471) {
          $472 = $470 | $468;
          HEAP32[(16440)>>2] = $472;
          HEAP32[$464>>2] = $350;
          $473 = ((($350)) + 24|0);
          HEAP32[$473>>2] = $464;
          $474 = ((($350)) + 12|0);
          HEAP32[$474>>2] = $350;
          $475 = ((($350)) + 8|0);
          HEAP32[$475>>2] = $350;
          break;
         }
         $476 = HEAP32[$464>>2]|0;
         $477 = ($$0361$i|0)==(31);
         $478 = $$0361$i >>> 1;
         $479 = (25 - ($478))|0;
         $480 = $477 ? 0 : $479;
         $481 = $$4351$lcssa$i << $480;
         $$0344$i = $481;$$0345$i = $476;
         while(1) {
          $482 = ((($$0345$i)) + 4|0);
          $483 = HEAP32[$482>>2]|0;
          $484 = $483 & -8;
          $485 = ($484|0)==($$4351$lcssa$i|0);
          if ($485) {
           label = 139;
           break;
          }
          $486 = $$0344$i >>> 31;
          $487 = (((($$0345$i)) + 16|0) + ($486<<2)|0);
          $488 = $$0344$i << 1;
          $489 = HEAP32[$487>>2]|0;
          $490 = ($489|0)==(0|0);
          if ($490) {
           label = 136;
           break;
          } else {
           $$0344$i = $488;$$0345$i = $489;
          }
         }
         if ((label|0) == 136) {
          $491 = HEAP32[(16452)>>2]|0;
          $492 = ($487>>>0)<($491>>>0);
          if ($492) {
           _abort();
           // unreachable;
          } else {
           HEAP32[$487>>2] = $350;
           $493 = ((($350)) + 24|0);
           HEAP32[$493>>2] = $$0345$i;
           $494 = ((($350)) + 12|0);
           HEAP32[$494>>2] = $350;
           $495 = ((($350)) + 8|0);
           HEAP32[$495>>2] = $350;
           break;
          }
         }
         else if ((label|0) == 139) {
          $496 = ((($$0345$i)) + 8|0);
          $497 = HEAP32[$496>>2]|0;
          $498 = HEAP32[(16452)>>2]|0;
          $499 = ($497>>>0)>=($498>>>0);
          $not$9$i = ($$0345$i>>>0)>=($498>>>0);
          $500 = $499 & $not$9$i;
          if ($500) {
           $501 = ((($497)) + 12|0);
           HEAP32[$501>>2] = $350;
           HEAP32[$496>>2] = $350;
           $502 = ((($350)) + 8|0);
           HEAP32[$502>>2] = $497;
           $503 = ((($350)) + 12|0);
           HEAP32[$503>>2] = $$0345$i;
           $504 = ((($350)) + 24|0);
           HEAP32[$504>>2] = 0;
           break;
          } else {
           _abort();
           // unreachable;
          }
         }
        }
       } while(0);
       $505 = ((($$4$lcssa$i)) + 8|0);
       $$0 = $505;
       STACKTOP = sp;return ($$0|0);
      } else {
       $$0197 = $249;
      }
     }
    }
   }
  }
 } while(0);
 $506 = HEAP32[(16444)>>2]|0;
 $507 = ($506>>>0)<($$0197>>>0);
 if (!($507)) {
  $508 = (($506) - ($$0197))|0;
  $509 = HEAP32[(16456)>>2]|0;
  $510 = ($508>>>0)>(15);
  if ($510) {
   $511 = (($509) + ($$0197)|0);
   HEAP32[(16456)>>2] = $511;
   HEAP32[(16444)>>2] = $508;
   $512 = $508 | 1;
   $513 = ((($511)) + 4|0);
   HEAP32[$513>>2] = $512;
   $514 = (($511) + ($508)|0);
   HEAP32[$514>>2] = $508;
   $515 = $$0197 | 3;
   $516 = ((($509)) + 4|0);
   HEAP32[$516>>2] = $515;
  } else {
   HEAP32[(16444)>>2] = 0;
   HEAP32[(16456)>>2] = 0;
   $517 = $506 | 3;
   $518 = ((($509)) + 4|0);
   HEAP32[$518>>2] = $517;
   $519 = (($509) + ($506)|0);
   $520 = ((($519)) + 4|0);
   $521 = HEAP32[$520>>2]|0;
   $522 = $521 | 1;
   HEAP32[$520>>2] = $522;
  }
  $523 = ((($509)) + 8|0);
  $$0 = $523;
  STACKTOP = sp;return ($$0|0);
 }
 $524 = HEAP32[(16448)>>2]|0;
 $525 = ($524>>>0)>($$0197>>>0);
 if ($525) {
  $526 = (($524) - ($$0197))|0;
  HEAP32[(16448)>>2] = $526;
  $527 = HEAP32[(16460)>>2]|0;
  $528 = (($527) + ($$0197)|0);
  HEAP32[(16460)>>2] = $528;
  $529 = $526 | 1;
  $530 = ((($528)) + 4|0);
  HEAP32[$530>>2] = $529;
  $531 = $$0197 | 3;
  $532 = ((($527)) + 4|0);
  HEAP32[$532>>2] = $531;
  $533 = ((($527)) + 8|0);
  $$0 = $533;
  STACKTOP = sp;return ($$0|0);
 }
 $534 = HEAP32[4227]|0;
 $535 = ($534|0)==(0);
 if ($535) {
  HEAP32[(16916)>>2] = 4096;
  HEAP32[(16912)>>2] = 4096;
  HEAP32[(16920)>>2] = -1;
  HEAP32[(16924)>>2] = -1;
  HEAP32[(16928)>>2] = 0;
  HEAP32[(16880)>>2] = 0;
  $536 = $1;
  $537 = $536 & -16;
  $538 = $537 ^ 1431655768;
  HEAP32[$1>>2] = $538;
  HEAP32[4227] = $538;
  $542 = 4096;
 } else {
  $$pre$i212 = HEAP32[(16916)>>2]|0;
  $542 = $$pre$i212;
 }
 $539 = (($$0197) + 48)|0;
 $540 = (($$0197) + 47)|0;
 $541 = (($542) + ($540))|0;
 $543 = (0 - ($542))|0;
 $544 = $541 & $543;
 $545 = ($544>>>0)>($$0197>>>0);
 if (!($545)) {
  $$0 = 0;
  STACKTOP = sp;return ($$0|0);
 }
 $546 = HEAP32[(16876)>>2]|0;
 $547 = ($546|0)==(0);
 if (!($547)) {
  $548 = HEAP32[(16868)>>2]|0;
  $549 = (($548) + ($544))|0;
  $550 = ($549>>>0)<=($548>>>0);
  $551 = ($549>>>0)>($546>>>0);
  $or$cond1$i = $550 | $551;
  if ($or$cond1$i) {
   $$0 = 0;
   STACKTOP = sp;return ($$0|0);
  }
 }
 $552 = HEAP32[(16880)>>2]|0;
 $553 = $552 & 4;
 $554 = ($553|0)==(0);
 L244: do {
  if ($554) {
   $555 = HEAP32[(16460)>>2]|0;
   $556 = ($555|0)==(0|0);
   L246: do {
    if ($556) {
     label = 163;
    } else {
     $$0$i$i = (16884);
     while(1) {
      $557 = HEAP32[$$0$i$i>>2]|0;
      $558 = ($557>>>0)>($555>>>0);
      if (!($558)) {
       $559 = ((($$0$i$i)) + 4|0);
       $560 = HEAP32[$559>>2]|0;
       $561 = (($557) + ($560)|0);
       $562 = ($561>>>0)>($555>>>0);
       if ($562) {
        break;
       }
      }
      $563 = ((($$0$i$i)) + 8|0);
      $564 = HEAP32[$563>>2]|0;
      $565 = ($564|0)==(0|0);
      if ($565) {
       label = 163;
       break L246;
      } else {
       $$0$i$i = $564;
      }
     }
     $588 = (($541) - ($524))|0;
     $589 = $588 & $543;
     $590 = ($589>>>0)<(2147483647);
     if ($590) {
      $591 = (_sbrk(($589|0))|0);
      $592 = HEAP32[$$0$i$i>>2]|0;
      $593 = HEAP32[$559>>2]|0;
      $594 = (($592) + ($593)|0);
      $595 = ($591|0)==($594|0);
      if ($595) {
       $596 = ($591|0)==((-1)|0);
       if ($596) {
        $$2234253237$i = $589;
       } else {
        $$723948$i = $589;$$749$i = $591;
        label = 180;
        break L244;
       }
      } else {
       $$2247$ph$i = $591;$$2253$ph$i = $589;
       label = 171;
      }
     } else {
      $$2234253237$i = 0;
     }
    }
   } while(0);
   do {
    if ((label|0) == 163) {
     $566 = (_sbrk(0)|0);
     $567 = ($566|0)==((-1)|0);
     if ($567) {
      $$2234253237$i = 0;
     } else {
      $568 = $566;
      $569 = HEAP32[(16912)>>2]|0;
      $570 = (($569) + -1)|0;
      $571 = $570 & $568;
      $572 = ($571|0)==(0);
      $573 = (($570) + ($568))|0;
      $574 = (0 - ($569))|0;
      $575 = $573 & $574;
      $576 = (($575) - ($568))|0;
      $577 = $572 ? 0 : $576;
      $$$i = (($577) + ($544))|0;
      $578 = HEAP32[(16868)>>2]|0;
      $579 = (($$$i) + ($578))|0;
      $580 = ($$$i>>>0)>($$0197>>>0);
      $581 = ($$$i>>>0)<(2147483647);
      $or$cond$i214 = $580 & $581;
      if ($or$cond$i214) {
       $582 = HEAP32[(16876)>>2]|0;
       $583 = ($582|0)==(0);
       if (!($583)) {
        $584 = ($579>>>0)<=($578>>>0);
        $585 = ($579>>>0)>($582>>>0);
        $or$cond2$i215 = $584 | $585;
        if ($or$cond2$i215) {
         $$2234253237$i = 0;
         break;
        }
       }
       $586 = (_sbrk(($$$i|0))|0);
       $587 = ($586|0)==($566|0);
       if ($587) {
        $$723948$i = $$$i;$$749$i = $566;
        label = 180;
        break L244;
       } else {
        $$2247$ph$i = $586;$$2253$ph$i = $$$i;
        label = 171;
       }
      } else {
       $$2234253237$i = 0;
      }
     }
    }
   } while(0);
   do {
    if ((label|0) == 171) {
     $597 = (0 - ($$2253$ph$i))|0;
     $598 = ($$2247$ph$i|0)!=((-1)|0);
     $599 = ($$2253$ph$i>>>0)<(2147483647);
     $or$cond7$i = $599 & $598;
     $600 = ($539>>>0)>($$2253$ph$i>>>0);
     $or$cond10$i = $600 & $or$cond7$i;
     if (!($or$cond10$i)) {
      $610 = ($$2247$ph$i|0)==((-1)|0);
      if ($610) {
       $$2234253237$i = 0;
       break;
      } else {
       $$723948$i = $$2253$ph$i;$$749$i = $$2247$ph$i;
       label = 180;
       break L244;
      }
     }
     $601 = HEAP32[(16916)>>2]|0;
     $602 = (($540) - ($$2253$ph$i))|0;
     $603 = (($602) + ($601))|0;
     $604 = (0 - ($601))|0;
     $605 = $603 & $604;
     $606 = ($605>>>0)<(2147483647);
     if (!($606)) {
      $$723948$i = $$2253$ph$i;$$749$i = $$2247$ph$i;
      label = 180;
      break L244;
     }
     $607 = (_sbrk(($605|0))|0);
     $608 = ($607|0)==((-1)|0);
     if ($608) {
      (_sbrk(($597|0))|0);
      $$2234253237$i = 0;
      break;
     } else {
      $609 = (($605) + ($$2253$ph$i))|0;
      $$723948$i = $609;$$749$i = $$2247$ph$i;
      label = 180;
      break L244;
     }
    }
   } while(0);
   $611 = HEAP32[(16880)>>2]|0;
   $612 = $611 | 4;
   HEAP32[(16880)>>2] = $612;
   $$4236$i = $$2234253237$i;
   label = 178;
  } else {
   $$4236$i = 0;
   label = 178;
  }
 } while(0);
 if ((label|0) == 178) {
  $613 = ($544>>>0)<(2147483647);
  if ($613) {
   $614 = (_sbrk(($544|0))|0);
   $615 = (_sbrk(0)|0);
   $616 = ($614|0)!=((-1)|0);
   $617 = ($615|0)!=((-1)|0);
   $or$cond5$i = $616 & $617;
   $618 = ($614>>>0)<($615>>>0);
   $or$cond11$i = $618 & $or$cond5$i;
   $619 = $615;
   $620 = $614;
   $621 = (($619) - ($620))|0;
   $622 = (($$0197) + 40)|0;
   $623 = ($621>>>0)>($622>>>0);
   $$$4236$i = $623 ? $621 : $$4236$i;
   $or$cond11$not$i = $or$cond11$i ^ 1;
   $624 = ($614|0)==((-1)|0);
   $not$$i216 = $623 ^ 1;
   $625 = $624 | $not$$i216;
   $or$cond50$i = $625 | $or$cond11$not$i;
   if (!($or$cond50$i)) {
    $$723948$i = $$$4236$i;$$749$i = $614;
    label = 180;
   }
  }
 }
 if ((label|0) == 180) {
  $626 = HEAP32[(16868)>>2]|0;
  $627 = (($626) + ($$723948$i))|0;
  HEAP32[(16868)>>2] = $627;
  $628 = HEAP32[(16872)>>2]|0;
  $629 = ($627>>>0)>($628>>>0);
  if ($629) {
   HEAP32[(16872)>>2] = $627;
  }
  $630 = HEAP32[(16460)>>2]|0;
  $631 = ($630|0)==(0|0);
  do {
   if ($631) {
    $632 = HEAP32[(16452)>>2]|0;
    $633 = ($632|0)==(0|0);
    $634 = ($$749$i>>>0)<($632>>>0);
    $or$cond12$i = $633 | $634;
    if ($or$cond12$i) {
     HEAP32[(16452)>>2] = $$749$i;
    }
    HEAP32[(16884)>>2] = $$749$i;
    HEAP32[(16888)>>2] = $$723948$i;
    HEAP32[(16896)>>2] = 0;
    $635 = HEAP32[4227]|0;
    HEAP32[(16472)>>2] = $635;
    HEAP32[(16468)>>2] = -1;
    $$01$i$i = 0;
    while(1) {
     $636 = $$01$i$i << 1;
     $637 = (16476 + ($636<<2)|0);
     $638 = ((($637)) + 12|0);
     HEAP32[$638>>2] = $637;
     $639 = ((($637)) + 8|0);
     HEAP32[$639>>2] = $637;
     $640 = (($$01$i$i) + 1)|0;
     $exitcond$i$i = ($640|0)==(32);
     if ($exitcond$i$i) {
      break;
     } else {
      $$01$i$i = $640;
     }
    }
    $641 = (($$723948$i) + -40)|0;
    $642 = ((($$749$i)) + 8|0);
    $643 = $642;
    $644 = $643 & 7;
    $645 = ($644|0)==(0);
    $646 = (0 - ($643))|0;
    $647 = $646 & 7;
    $648 = $645 ? 0 : $647;
    $649 = (($$749$i) + ($648)|0);
    $650 = (($641) - ($648))|0;
    HEAP32[(16460)>>2] = $649;
    HEAP32[(16448)>>2] = $650;
    $651 = $650 | 1;
    $652 = ((($649)) + 4|0);
    HEAP32[$652>>2] = $651;
    $653 = (($649) + ($650)|0);
    $654 = ((($653)) + 4|0);
    HEAP32[$654>>2] = 40;
    $655 = HEAP32[(16924)>>2]|0;
    HEAP32[(16464)>>2] = $655;
   } else {
    $$024371$i = (16884);
    while(1) {
     $656 = HEAP32[$$024371$i>>2]|0;
     $657 = ((($$024371$i)) + 4|0);
     $658 = HEAP32[$657>>2]|0;
     $659 = (($656) + ($658)|0);
     $660 = ($$749$i|0)==($659|0);
     if ($660) {
      label = 190;
      break;
     }
     $661 = ((($$024371$i)) + 8|0);
     $662 = HEAP32[$661>>2]|0;
     $663 = ($662|0)==(0|0);
     if ($663) {
      break;
     } else {
      $$024371$i = $662;
     }
    }
    if ((label|0) == 190) {
     $664 = ((($$024371$i)) + 12|0);
     $665 = HEAP32[$664>>2]|0;
     $666 = $665 & 8;
     $667 = ($666|0)==(0);
     if ($667) {
      $668 = ($630>>>0)>=($656>>>0);
      $669 = ($630>>>0)<($$749$i>>>0);
      $or$cond51$i = $669 & $668;
      if ($or$cond51$i) {
       $670 = (($658) + ($$723948$i))|0;
       HEAP32[$657>>2] = $670;
       $671 = HEAP32[(16448)>>2]|0;
       $672 = ((($630)) + 8|0);
       $673 = $672;
       $674 = $673 & 7;
       $675 = ($674|0)==(0);
       $676 = (0 - ($673))|0;
       $677 = $676 & 7;
       $678 = $675 ? 0 : $677;
       $679 = (($630) + ($678)|0);
       $680 = (($$723948$i) - ($678))|0;
       $681 = (($671) + ($680))|0;
       HEAP32[(16460)>>2] = $679;
       HEAP32[(16448)>>2] = $681;
       $682 = $681 | 1;
       $683 = ((($679)) + 4|0);
       HEAP32[$683>>2] = $682;
       $684 = (($679) + ($681)|0);
       $685 = ((($684)) + 4|0);
       HEAP32[$685>>2] = 40;
       $686 = HEAP32[(16924)>>2]|0;
       HEAP32[(16464)>>2] = $686;
       break;
      }
     }
    }
    $687 = HEAP32[(16452)>>2]|0;
    $688 = ($$749$i>>>0)<($687>>>0);
    if ($688) {
     HEAP32[(16452)>>2] = $$749$i;
     $752 = $$749$i;
    } else {
     $752 = $687;
    }
    $689 = (($$749$i) + ($$723948$i)|0);
    $$124470$i = (16884);
    while(1) {
     $690 = HEAP32[$$124470$i>>2]|0;
     $691 = ($690|0)==($689|0);
     if ($691) {
      label = 198;
      break;
     }
     $692 = ((($$124470$i)) + 8|0);
     $693 = HEAP32[$692>>2]|0;
     $694 = ($693|0)==(0|0);
     if ($694) {
      break;
     } else {
      $$124470$i = $693;
     }
    }
    if ((label|0) == 198) {
     $695 = ((($$124470$i)) + 12|0);
     $696 = HEAP32[$695>>2]|0;
     $697 = $696 & 8;
     $698 = ($697|0)==(0);
     if ($698) {
      HEAP32[$$124470$i>>2] = $$749$i;
      $699 = ((($$124470$i)) + 4|0);
      $700 = HEAP32[$699>>2]|0;
      $701 = (($700) + ($$723948$i))|0;
      HEAP32[$699>>2] = $701;
      $702 = ((($$749$i)) + 8|0);
      $703 = $702;
      $704 = $703 & 7;
      $705 = ($704|0)==(0);
      $706 = (0 - ($703))|0;
      $707 = $706 & 7;
      $708 = $705 ? 0 : $707;
      $709 = (($$749$i) + ($708)|0);
      $710 = ((($689)) + 8|0);
      $711 = $710;
      $712 = $711 & 7;
      $713 = ($712|0)==(0);
      $714 = (0 - ($711))|0;
      $715 = $714 & 7;
      $716 = $713 ? 0 : $715;
      $717 = (($689) + ($716)|0);
      $718 = $717;
      $719 = $709;
      $720 = (($718) - ($719))|0;
      $721 = (($709) + ($$0197)|0);
      $722 = (($720) - ($$0197))|0;
      $723 = $$0197 | 3;
      $724 = ((($709)) + 4|0);
      HEAP32[$724>>2] = $723;
      $725 = ($717|0)==($630|0);
      do {
       if ($725) {
        $726 = HEAP32[(16448)>>2]|0;
        $727 = (($726) + ($722))|0;
        HEAP32[(16448)>>2] = $727;
        HEAP32[(16460)>>2] = $721;
        $728 = $727 | 1;
        $729 = ((($721)) + 4|0);
        HEAP32[$729>>2] = $728;
       } else {
        $730 = HEAP32[(16456)>>2]|0;
        $731 = ($717|0)==($730|0);
        if ($731) {
         $732 = HEAP32[(16444)>>2]|0;
         $733 = (($732) + ($722))|0;
         HEAP32[(16444)>>2] = $733;
         HEAP32[(16456)>>2] = $721;
         $734 = $733 | 1;
         $735 = ((($721)) + 4|0);
         HEAP32[$735>>2] = $734;
         $736 = (($721) + ($733)|0);
         HEAP32[$736>>2] = $733;
         break;
        }
        $737 = ((($717)) + 4|0);
        $738 = HEAP32[$737>>2]|0;
        $739 = $738 & 3;
        $740 = ($739|0)==(1);
        if ($740) {
         $741 = $738 & -8;
         $742 = $738 >>> 3;
         $743 = ($738>>>0)<(256);
         L314: do {
          if ($743) {
           $744 = ((($717)) + 8|0);
           $745 = HEAP32[$744>>2]|0;
           $746 = ((($717)) + 12|0);
           $747 = HEAP32[$746>>2]|0;
           $748 = $742 << 1;
           $749 = (16476 + ($748<<2)|0);
           $750 = ($745|0)==($749|0);
           do {
            if (!($750)) {
             $751 = ($745>>>0)<($752>>>0);
             if ($751) {
              _abort();
              // unreachable;
             }
             $753 = ((($745)) + 12|0);
             $754 = HEAP32[$753>>2]|0;
             $755 = ($754|0)==($717|0);
             if ($755) {
              break;
             }
             _abort();
             // unreachable;
            }
           } while(0);
           $756 = ($747|0)==($745|0);
           if ($756) {
            $757 = 1 << $742;
            $758 = $757 ^ -1;
            $759 = HEAP32[4109]|0;
            $760 = $759 & $758;
            HEAP32[4109] = $760;
            break;
           }
           $761 = ($747|0)==($749|0);
           do {
            if ($761) {
             $$pre10$i$i = ((($747)) + 8|0);
             $$pre$phi11$i$iZ2D = $$pre10$i$i;
            } else {
             $762 = ($747>>>0)<($752>>>0);
             if ($762) {
              _abort();
              // unreachable;
             }
             $763 = ((($747)) + 8|0);
             $764 = HEAP32[$763>>2]|0;
             $765 = ($764|0)==($717|0);
             if ($765) {
              $$pre$phi11$i$iZ2D = $763;
              break;
             }
             _abort();
             // unreachable;
            }
           } while(0);
           $766 = ((($745)) + 12|0);
           HEAP32[$766>>2] = $747;
           HEAP32[$$pre$phi11$i$iZ2D>>2] = $745;
          } else {
           $767 = ((($717)) + 24|0);
           $768 = HEAP32[$767>>2]|0;
           $769 = ((($717)) + 12|0);
           $770 = HEAP32[$769>>2]|0;
           $771 = ($770|0)==($717|0);
           do {
            if ($771) {
             $781 = ((($717)) + 16|0);
             $782 = ((($781)) + 4|0);
             $783 = HEAP32[$782>>2]|0;
             $784 = ($783|0)==(0|0);
             if ($784) {
              $785 = HEAP32[$781>>2]|0;
              $786 = ($785|0)==(0|0);
              if ($786) {
               $$3$i$i = 0;
               break;
              } else {
               $$1291$i$i = $785;$$1293$i$i = $781;
              }
             } else {
              $$1291$i$i = $783;$$1293$i$i = $782;
             }
             while(1) {
              $787 = ((($$1291$i$i)) + 20|0);
              $788 = HEAP32[$787>>2]|0;
              $789 = ($788|0)==(0|0);
              if (!($789)) {
               $$1291$i$i = $788;$$1293$i$i = $787;
               continue;
              }
              $790 = ((($$1291$i$i)) + 16|0);
              $791 = HEAP32[$790>>2]|0;
              $792 = ($791|0)==(0|0);
              if ($792) {
               break;
              } else {
               $$1291$i$i = $791;$$1293$i$i = $790;
              }
             }
             $793 = ($$1293$i$i>>>0)<($752>>>0);
             if ($793) {
              _abort();
              // unreachable;
             } else {
              HEAP32[$$1293$i$i>>2] = 0;
              $$3$i$i = $$1291$i$i;
              break;
             }
            } else {
             $772 = ((($717)) + 8|0);
             $773 = HEAP32[$772>>2]|0;
             $774 = ($773>>>0)<($752>>>0);
             if ($774) {
              _abort();
              // unreachable;
             }
             $775 = ((($773)) + 12|0);
             $776 = HEAP32[$775>>2]|0;
             $777 = ($776|0)==($717|0);
             if (!($777)) {
              _abort();
              // unreachable;
             }
             $778 = ((($770)) + 8|0);
             $779 = HEAP32[$778>>2]|0;
             $780 = ($779|0)==($717|0);
             if ($780) {
              HEAP32[$775>>2] = $770;
              HEAP32[$778>>2] = $773;
              $$3$i$i = $770;
              break;
             } else {
              _abort();
              // unreachable;
             }
            }
           } while(0);
           $794 = ($768|0)==(0|0);
           if ($794) {
            break;
           }
           $795 = ((($717)) + 28|0);
           $796 = HEAP32[$795>>2]|0;
           $797 = (16740 + ($796<<2)|0);
           $798 = HEAP32[$797>>2]|0;
           $799 = ($717|0)==($798|0);
           do {
            if ($799) {
             HEAP32[$797>>2] = $$3$i$i;
             $cond$i$i = ($$3$i$i|0)==(0|0);
             if (!($cond$i$i)) {
              break;
             }
             $800 = 1 << $796;
             $801 = $800 ^ -1;
             $802 = HEAP32[(16440)>>2]|0;
             $803 = $802 & $801;
             HEAP32[(16440)>>2] = $803;
             break L314;
            } else {
             $804 = HEAP32[(16452)>>2]|0;
             $805 = ($768>>>0)<($804>>>0);
             if ($805) {
              _abort();
              // unreachable;
             } else {
              $806 = ((($768)) + 16|0);
              $807 = HEAP32[$806>>2]|0;
              $not$$i17$i = ($807|0)!=($717|0);
              $$sink1$i$i = $not$$i17$i&1;
              $808 = (((($768)) + 16|0) + ($$sink1$i$i<<2)|0);
              HEAP32[$808>>2] = $$3$i$i;
              $809 = ($$3$i$i|0)==(0|0);
              if ($809) {
               break L314;
              } else {
               break;
              }
             }
            }
           } while(0);
           $810 = HEAP32[(16452)>>2]|0;
           $811 = ($$3$i$i>>>0)<($810>>>0);
           if ($811) {
            _abort();
            // unreachable;
           }
           $812 = ((($$3$i$i)) + 24|0);
           HEAP32[$812>>2] = $768;
           $813 = ((($717)) + 16|0);
           $814 = HEAP32[$813>>2]|0;
           $815 = ($814|0)==(0|0);
           do {
            if (!($815)) {
             $816 = ($814>>>0)<($810>>>0);
             if ($816) {
              _abort();
              // unreachable;
             } else {
              $817 = ((($$3$i$i)) + 16|0);
              HEAP32[$817>>2] = $814;
              $818 = ((($814)) + 24|0);
              HEAP32[$818>>2] = $$3$i$i;
              break;
             }
            }
           } while(0);
           $819 = ((($813)) + 4|0);
           $820 = HEAP32[$819>>2]|0;
           $821 = ($820|0)==(0|0);
           if ($821) {
            break;
           }
           $822 = HEAP32[(16452)>>2]|0;
           $823 = ($820>>>0)<($822>>>0);
           if ($823) {
            _abort();
            // unreachable;
           } else {
            $824 = ((($$3$i$i)) + 20|0);
            HEAP32[$824>>2] = $820;
            $825 = ((($820)) + 24|0);
            HEAP32[$825>>2] = $$3$i$i;
            break;
           }
          }
         } while(0);
         $826 = (($717) + ($741)|0);
         $827 = (($741) + ($722))|0;
         $$0$i18$i = $826;$$0287$i$i = $827;
        } else {
         $$0$i18$i = $717;$$0287$i$i = $722;
        }
        $828 = ((($$0$i18$i)) + 4|0);
        $829 = HEAP32[$828>>2]|0;
        $830 = $829 & -2;
        HEAP32[$828>>2] = $830;
        $831 = $$0287$i$i | 1;
        $832 = ((($721)) + 4|0);
        HEAP32[$832>>2] = $831;
        $833 = (($721) + ($$0287$i$i)|0);
        HEAP32[$833>>2] = $$0287$i$i;
        $834 = $$0287$i$i >>> 3;
        $835 = ($$0287$i$i>>>0)<(256);
        if ($835) {
         $836 = $834 << 1;
         $837 = (16476 + ($836<<2)|0);
         $838 = HEAP32[4109]|0;
         $839 = 1 << $834;
         $840 = $838 & $839;
         $841 = ($840|0)==(0);
         do {
          if ($841) {
           $842 = $838 | $839;
           HEAP32[4109] = $842;
           $$pre$i19$i = ((($837)) + 8|0);
           $$0295$i$i = $837;$$pre$phi$i20$iZ2D = $$pre$i19$i;
          } else {
           $843 = ((($837)) + 8|0);
           $844 = HEAP32[$843>>2]|0;
           $845 = HEAP32[(16452)>>2]|0;
           $846 = ($844>>>0)<($845>>>0);
           if (!($846)) {
            $$0295$i$i = $844;$$pre$phi$i20$iZ2D = $843;
            break;
           }
           _abort();
           // unreachable;
          }
         } while(0);
         HEAP32[$$pre$phi$i20$iZ2D>>2] = $721;
         $847 = ((($$0295$i$i)) + 12|0);
         HEAP32[$847>>2] = $721;
         $848 = ((($721)) + 8|0);
         HEAP32[$848>>2] = $$0295$i$i;
         $849 = ((($721)) + 12|0);
         HEAP32[$849>>2] = $837;
         break;
        }
        $850 = $$0287$i$i >>> 8;
        $851 = ($850|0)==(0);
        do {
         if ($851) {
          $$0296$i$i = 0;
         } else {
          $852 = ($$0287$i$i>>>0)>(16777215);
          if ($852) {
           $$0296$i$i = 31;
           break;
          }
          $853 = (($850) + 1048320)|0;
          $854 = $853 >>> 16;
          $855 = $854 & 8;
          $856 = $850 << $855;
          $857 = (($856) + 520192)|0;
          $858 = $857 >>> 16;
          $859 = $858 & 4;
          $860 = $859 | $855;
          $861 = $856 << $859;
          $862 = (($861) + 245760)|0;
          $863 = $862 >>> 16;
          $864 = $863 & 2;
          $865 = $860 | $864;
          $866 = (14 - ($865))|0;
          $867 = $861 << $864;
          $868 = $867 >>> 15;
          $869 = (($866) + ($868))|0;
          $870 = $869 << 1;
          $871 = (($869) + 7)|0;
          $872 = $$0287$i$i >>> $871;
          $873 = $872 & 1;
          $874 = $873 | $870;
          $$0296$i$i = $874;
         }
        } while(0);
        $875 = (16740 + ($$0296$i$i<<2)|0);
        $876 = ((($721)) + 28|0);
        HEAP32[$876>>2] = $$0296$i$i;
        $877 = ((($721)) + 16|0);
        $878 = ((($877)) + 4|0);
        HEAP32[$878>>2] = 0;
        HEAP32[$877>>2] = 0;
        $879 = HEAP32[(16440)>>2]|0;
        $880 = 1 << $$0296$i$i;
        $881 = $879 & $880;
        $882 = ($881|0)==(0);
        if ($882) {
         $883 = $879 | $880;
         HEAP32[(16440)>>2] = $883;
         HEAP32[$875>>2] = $721;
         $884 = ((($721)) + 24|0);
         HEAP32[$884>>2] = $875;
         $885 = ((($721)) + 12|0);
         HEAP32[$885>>2] = $721;
         $886 = ((($721)) + 8|0);
         HEAP32[$886>>2] = $721;
         break;
        }
        $887 = HEAP32[$875>>2]|0;
        $888 = ($$0296$i$i|0)==(31);
        $889 = $$0296$i$i >>> 1;
        $890 = (25 - ($889))|0;
        $891 = $888 ? 0 : $890;
        $892 = $$0287$i$i << $891;
        $$0288$i$i = $892;$$0289$i$i = $887;
        while(1) {
         $893 = ((($$0289$i$i)) + 4|0);
         $894 = HEAP32[$893>>2]|0;
         $895 = $894 & -8;
         $896 = ($895|0)==($$0287$i$i|0);
         if ($896) {
          label = 265;
          break;
         }
         $897 = $$0288$i$i >>> 31;
         $898 = (((($$0289$i$i)) + 16|0) + ($897<<2)|0);
         $899 = $$0288$i$i << 1;
         $900 = HEAP32[$898>>2]|0;
         $901 = ($900|0)==(0|0);
         if ($901) {
          label = 262;
          break;
         } else {
          $$0288$i$i = $899;$$0289$i$i = $900;
         }
        }
        if ((label|0) == 262) {
         $902 = HEAP32[(16452)>>2]|0;
         $903 = ($898>>>0)<($902>>>0);
         if ($903) {
          _abort();
          // unreachable;
         } else {
          HEAP32[$898>>2] = $721;
          $904 = ((($721)) + 24|0);
          HEAP32[$904>>2] = $$0289$i$i;
          $905 = ((($721)) + 12|0);
          HEAP32[$905>>2] = $721;
          $906 = ((($721)) + 8|0);
          HEAP32[$906>>2] = $721;
          break;
         }
        }
        else if ((label|0) == 265) {
         $907 = ((($$0289$i$i)) + 8|0);
         $908 = HEAP32[$907>>2]|0;
         $909 = HEAP32[(16452)>>2]|0;
         $910 = ($908>>>0)>=($909>>>0);
         $not$7$i$i = ($$0289$i$i>>>0)>=($909>>>0);
         $911 = $910 & $not$7$i$i;
         if ($911) {
          $912 = ((($908)) + 12|0);
          HEAP32[$912>>2] = $721;
          HEAP32[$907>>2] = $721;
          $913 = ((($721)) + 8|0);
          HEAP32[$913>>2] = $908;
          $914 = ((($721)) + 12|0);
          HEAP32[$914>>2] = $$0289$i$i;
          $915 = ((($721)) + 24|0);
          HEAP32[$915>>2] = 0;
          break;
         } else {
          _abort();
          // unreachable;
         }
        }
       }
      } while(0);
      $1047 = ((($709)) + 8|0);
      $$0 = $1047;
      STACKTOP = sp;return ($$0|0);
     }
    }
    $$0$i$i$i = (16884);
    while(1) {
     $916 = HEAP32[$$0$i$i$i>>2]|0;
     $917 = ($916>>>0)>($630>>>0);
     if (!($917)) {
      $918 = ((($$0$i$i$i)) + 4|0);
      $919 = HEAP32[$918>>2]|0;
      $920 = (($916) + ($919)|0);
      $921 = ($920>>>0)>($630>>>0);
      if ($921) {
       break;
      }
     }
     $922 = ((($$0$i$i$i)) + 8|0);
     $923 = HEAP32[$922>>2]|0;
     $$0$i$i$i = $923;
    }
    $924 = ((($920)) + -47|0);
    $925 = ((($924)) + 8|0);
    $926 = $925;
    $927 = $926 & 7;
    $928 = ($927|0)==(0);
    $929 = (0 - ($926))|0;
    $930 = $929 & 7;
    $931 = $928 ? 0 : $930;
    $932 = (($924) + ($931)|0);
    $933 = ((($630)) + 16|0);
    $934 = ($932>>>0)<($933>>>0);
    $935 = $934 ? $630 : $932;
    $936 = ((($935)) + 8|0);
    $937 = ((($935)) + 24|0);
    $938 = (($$723948$i) + -40)|0;
    $939 = ((($$749$i)) + 8|0);
    $940 = $939;
    $941 = $940 & 7;
    $942 = ($941|0)==(0);
    $943 = (0 - ($940))|0;
    $944 = $943 & 7;
    $945 = $942 ? 0 : $944;
    $946 = (($$749$i) + ($945)|0);
    $947 = (($938) - ($945))|0;
    HEAP32[(16460)>>2] = $946;
    HEAP32[(16448)>>2] = $947;
    $948 = $947 | 1;
    $949 = ((($946)) + 4|0);
    HEAP32[$949>>2] = $948;
    $950 = (($946) + ($947)|0);
    $951 = ((($950)) + 4|0);
    HEAP32[$951>>2] = 40;
    $952 = HEAP32[(16924)>>2]|0;
    HEAP32[(16464)>>2] = $952;
    $953 = ((($935)) + 4|0);
    HEAP32[$953>>2] = 27;
    ;HEAP32[$936>>2]=HEAP32[(16884)>>2]|0;HEAP32[$936+4>>2]=HEAP32[(16884)+4>>2]|0;HEAP32[$936+8>>2]=HEAP32[(16884)+8>>2]|0;HEAP32[$936+12>>2]=HEAP32[(16884)+12>>2]|0;
    HEAP32[(16884)>>2] = $$749$i;
    HEAP32[(16888)>>2] = $$723948$i;
    HEAP32[(16896)>>2] = 0;
    HEAP32[(16892)>>2] = $936;
    $955 = $937;
    while(1) {
     $954 = ((($955)) + 4|0);
     HEAP32[$954>>2] = 7;
     $956 = ((($955)) + 8|0);
     $957 = ($956>>>0)<($920>>>0);
     if ($957) {
      $955 = $954;
     } else {
      break;
     }
    }
    $958 = ($935|0)==($630|0);
    if (!($958)) {
     $959 = $935;
     $960 = $630;
     $961 = (($959) - ($960))|0;
     $962 = HEAP32[$953>>2]|0;
     $963 = $962 & -2;
     HEAP32[$953>>2] = $963;
     $964 = $961 | 1;
     $965 = ((($630)) + 4|0);
     HEAP32[$965>>2] = $964;
     HEAP32[$935>>2] = $961;
     $966 = $961 >>> 3;
     $967 = ($961>>>0)<(256);
     if ($967) {
      $968 = $966 << 1;
      $969 = (16476 + ($968<<2)|0);
      $970 = HEAP32[4109]|0;
      $971 = 1 << $966;
      $972 = $970 & $971;
      $973 = ($972|0)==(0);
      if ($973) {
       $974 = $970 | $971;
       HEAP32[4109] = $974;
       $$pre$i$i = ((($969)) + 8|0);
       $$0211$i$i = $969;$$pre$phi$i$iZ2D = $$pre$i$i;
      } else {
       $975 = ((($969)) + 8|0);
       $976 = HEAP32[$975>>2]|0;
       $977 = HEAP32[(16452)>>2]|0;
       $978 = ($976>>>0)<($977>>>0);
       if ($978) {
        _abort();
        // unreachable;
       } else {
        $$0211$i$i = $976;$$pre$phi$i$iZ2D = $975;
       }
      }
      HEAP32[$$pre$phi$i$iZ2D>>2] = $630;
      $979 = ((($$0211$i$i)) + 12|0);
      HEAP32[$979>>2] = $630;
      $980 = ((($630)) + 8|0);
      HEAP32[$980>>2] = $$0211$i$i;
      $981 = ((($630)) + 12|0);
      HEAP32[$981>>2] = $969;
      break;
     }
     $982 = $961 >>> 8;
     $983 = ($982|0)==(0);
     if ($983) {
      $$0212$i$i = 0;
     } else {
      $984 = ($961>>>0)>(16777215);
      if ($984) {
       $$0212$i$i = 31;
      } else {
       $985 = (($982) + 1048320)|0;
       $986 = $985 >>> 16;
       $987 = $986 & 8;
       $988 = $982 << $987;
       $989 = (($988) + 520192)|0;
       $990 = $989 >>> 16;
       $991 = $990 & 4;
       $992 = $991 | $987;
       $993 = $988 << $991;
       $994 = (($993) + 245760)|0;
       $995 = $994 >>> 16;
       $996 = $995 & 2;
       $997 = $992 | $996;
       $998 = (14 - ($997))|0;
       $999 = $993 << $996;
       $1000 = $999 >>> 15;
       $1001 = (($998) + ($1000))|0;
       $1002 = $1001 << 1;
       $1003 = (($1001) + 7)|0;
       $1004 = $961 >>> $1003;
       $1005 = $1004 & 1;
       $1006 = $1005 | $1002;
       $$0212$i$i = $1006;
      }
     }
     $1007 = (16740 + ($$0212$i$i<<2)|0);
     $1008 = ((($630)) + 28|0);
     HEAP32[$1008>>2] = $$0212$i$i;
     $1009 = ((($630)) + 20|0);
     HEAP32[$1009>>2] = 0;
     HEAP32[$933>>2] = 0;
     $1010 = HEAP32[(16440)>>2]|0;
     $1011 = 1 << $$0212$i$i;
     $1012 = $1010 & $1011;
     $1013 = ($1012|0)==(0);
     if ($1013) {
      $1014 = $1010 | $1011;
      HEAP32[(16440)>>2] = $1014;
      HEAP32[$1007>>2] = $630;
      $1015 = ((($630)) + 24|0);
      HEAP32[$1015>>2] = $1007;
      $1016 = ((($630)) + 12|0);
      HEAP32[$1016>>2] = $630;
      $1017 = ((($630)) + 8|0);
      HEAP32[$1017>>2] = $630;
      break;
     }
     $1018 = HEAP32[$1007>>2]|0;
     $1019 = ($$0212$i$i|0)==(31);
     $1020 = $$0212$i$i >>> 1;
     $1021 = (25 - ($1020))|0;
     $1022 = $1019 ? 0 : $1021;
     $1023 = $961 << $1022;
     $$0206$i$i = $1023;$$0207$i$i = $1018;
     while(1) {
      $1024 = ((($$0207$i$i)) + 4|0);
      $1025 = HEAP32[$1024>>2]|0;
      $1026 = $1025 & -8;
      $1027 = ($1026|0)==($961|0);
      if ($1027) {
       label = 292;
       break;
      }
      $1028 = $$0206$i$i >>> 31;
      $1029 = (((($$0207$i$i)) + 16|0) + ($1028<<2)|0);
      $1030 = $$0206$i$i << 1;
      $1031 = HEAP32[$1029>>2]|0;
      $1032 = ($1031|0)==(0|0);
      if ($1032) {
       label = 289;
       break;
      } else {
       $$0206$i$i = $1030;$$0207$i$i = $1031;
      }
     }
     if ((label|0) == 289) {
      $1033 = HEAP32[(16452)>>2]|0;
      $1034 = ($1029>>>0)<($1033>>>0);
      if ($1034) {
       _abort();
       // unreachable;
      } else {
       HEAP32[$1029>>2] = $630;
       $1035 = ((($630)) + 24|0);
       HEAP32[$1035>>2] = $$0207$i$i;
       $1036 = ((($630)) + 12|0);
       HEAP32[$1036>>2] = $630;
       $1037 = ((($630)) + 8|0);
       HEAP32[$1037>>2] = $630;
       break;
      }
     }
     else if ((label|0) == 292) {
      $1038 = ((($$0207$i$i)) + 8|0);
      $1039 = HEAP32[$1038>>2]|0;
      $1040 = HEAP32[(16452)>>2]|0;
      $1041 = ($1039>>>0)>=($1040>>>0);
      $not$$i$i = ($$0207$i$i>>>0)>=($1040>>>0);
      $1042 = $1041 & $not$$i$i;
      if ($1042) {
       $1043 = ((($1039)) + 12|0);
       HEAP32[$1043>>2] = $630;
       HEAP32[$1038>>2] = $630;
       $1044 = ((($630)) + 8|0);
       HEAP32[$1044>>2] = $1039;
       $1045 = ((($630)) + 12|0);
       HEAP32[$1045>>2] = $$0207$i$i;
       $1046 = ((($630)) + 24|0);
       HEAP32[$1046>>2] = 0;
       break;
      } else {
       _abort();
       // unreachable;
      }
     }
    }
   }
  } while(0);
  $1048 = HEAP32[(16448)>>2]|0;
  $1049 = ($1048>>>0)>($$0197>>>0);
  if ($1049) {
   $1050 = (($1048) - ($$0197))|0;
   HEAP32[(16448)>>2] = $1050;
   $1051 = HEAP32[(16460)>>2]|0;
   $1052 = (($1051) + ($$0197)|0);
   HEAP32[(16460)>>2] = $1052;
   $1053 = $1050 | 1;
   $1054 = ((($1052)) + 4|0);
   HEAP32[$1054>>2] = $1053;
   $1055 = $$0197 | 3;
   $1056 = ((($1051)) + 4|0);
   HEAP32[$1056>>2] = $1055;
   $1057 = ((($1051)) + 8|0);
   $$0 = $1057;
   STACKTOP = sp;return ($$0|0);
  }
 }
 $1058 = (___errno_location()|0);
 HEAP32[$1058>>2] = 12;
 $$0 = 0;
 STACKTOP = sp;return ($$0|0);
}
function _free($0) {
 $0 = $0|0;
 var $$0212$i = 0, $$0212$in$i = 0, $$0383 = 0, $$0384 = 0, $$0396 = 0, $$0403 = 0, $$1 = 0, $$1382 = 0, $$1387 = 0, $$1390 = 0, $$1398 = 0, $$1402 = 0, $$2 = 0, $$3 = 0, $$3400 = 0, $$pre = 0, $$pre$phi443Z2D = 0, $$pre$phi445Z2D = 0, $$pre$phiZ2D = 0, $$pre442 = 0;
 var $$pre444 = 0, $$sink3 = 0, $$sink5 = 0, $1 = 0, $10 = 0, $100 = 0, $101 = 0, $102 = 0, $103 = 0, $104 = 0, $105 = 0, $106 = 0, $107 = 0, $108 = 0, $109 = 0, $11 = 0, $110 = 0, $111 = 0, $112 = 0, $113 = 0;
 var $114 = 0, $115 = 0, $116 = 0, $117 = 0, $118 = 0, $119 = 0, $12 = 0, $120 = 0, $121 = 0, $122 = 0, $123 = 0, $124 = 0, $125 = 0, $126 = 0, $127 = 0, $128 = 0, $129 = 0, $13 = 0, $130 = 0, $131 = 0;
 var $132 = 0, $133 = 0, $134 = 0, $135 = 0, $136 = 0, $137 = 0, $138 = 0, $139 = 0, $14 = 0, $140 = 0, $141 = 0, $142 = 0, $143 = 0, $144 = 0, $145 = 0, $146 = 0, $147 = 0, $148 = 0, $149 = 0, $15 = 0;
 var $150 = 0, $151 = 0, $152 = 0, $153 = 0, $154 = 0, $155 = 0, $156 = 0, $157 = 0, $158 = 0, $159 = 0, $16 = 0, $160 = 0, $161 = 0, $162 = 0, $163 = 0, $164 = 0, $165 = 0, $166 = 0, $167 = 0, $168 = 0;
 var $169 = 0, $17 = 0, $170 = 0, $171 = 0, $172 = 0, $173 = 0, $174 = 0, $175 = 0, $176 = 0, $177 = 0, $178 = 0, $179 = 0, $18 = 0, $180 = 0, $181 = 0, $182 = 0, $183 = 0, $184 = 0, $185 = 0, $186 = 0;
 var $187 = 0, $188 = 0, $189 = 0, $19 = 0, $190 = 0, $191 = 0, $192 = 0, $193 = 0, $194 = 0, $195 = 0, $196 = 0, $197 = 0, $198 = 0, $199 = 0, $2 = 0, $20 = 0, $200 = 0, $201 = 0, $202 = 0, $203 = 0;
 var $204 = 0, $205 = 0, $206 = 0, $207 = 0, $208 = 0, $209 = 0, $21 = 0, $210 = 0, $211 = 0, $212 = 0, $213 = 0, $214 = 0, $215 = 0, $216 = 0, $217 = 0, $218 = 0, $219 = 0, $22 = 0, $220 = 0, $221 = 0;
 var $222 = 0, $223 = 0, $224 = 0, $225 = 0, $226 = 0, $227 = 0, $228 = 0, $229 = 0, $23 = 0, $230 = 0, $231 = 0, $232 = 0, $233 = 0, $234 = 0, $235 = 0, $236 = 0, $237 = 0, $238 = 0, $239 = 0, $24 = 0;
 var $240 = 0, $241 = 0, $242 = 0, $243 = 0, $244 = 0, $245 = 0, $246 = 0, $247 = 0, $248 = 0, $249 = 0, $25 = 0, $250 = 0, $251 = 0, $252 = 0, $253 = 0, $254 = 0, $255 = 0, $256 = 0, $257 = 0, $258 = 0;
 var $259 = 0, $26 = 0, $260 = 0, $261 = 0, $262 = 0, $263 = 0, $264 = 0, $265 = 0, $266 = 0, $267 = 0, $268 = 0, $269 = 0, $27 = 0, $270 = 0, $271 = 0, $272 = 0, $273 = 0, $274 = 0, $275 = 0, $276 = 0;
 var $277 = 0, $278 = 0, $279 = 0, $28 = 0, $280 = 0, $281 = 0, $282 = 0, $283 = 0, $284 = 0, $285 = 0, $286 = 0, $287 = 0, $288 = 0, $289 = 0, $29 = 0, $290 = 0, $291 = 0, $292 = 0, $293 = 0, $294 = 0;
 var $295 = 0, $296 = 0, $297 = 0, $298 = 0, $299 = 0, $3 = 0, $30 = 0, $300 = 0, $301 = 0, $302 = 0, $303 = 0, $304 = 0, $305 = 0, $306 = 0, $307 = 0, $308 = 0, $309 = 0, $31 = 0, $310 = 0, $311 = 0;
 var $312 = 0, $313 = 0, $314 = 0, $315 = 0, $316 = 0, $317 = 0, $318 = 0, $32 = 0, $33 = 0, $34 = 0, $35 = 0, $36 = 0, $37 = 0, $38 = 0, $39 = 0, $4 = 0, $40 = 0, $41 = 0, $42 = 0, $43 = 0;
 var $44 = 0, $45 = 0, $46 = 0, $47 = 0, $48 = 0, $49 = 0, $5 = 0, $50 = 0, $51 = 0, $52 = 0, $53 = 0, $54 = 0, $55 = 0, $56 = 0, $57 = 0, $58 = 0, $59 = 0, $6 = 0, $60 = 0, $61 = 0;
 var $62 = 0, $63 = 0, $64 = 0, $65 = 0, $66 = 0, $67 = 0, $68 = 0, $69 = 0, $7 = 0, $70 = 0, $71 = 0, $72 = 0, $73 = 0, $74 = 0, $75 = 0, $76 = 0, $77 = 0, $78 = 0, $79 = 0, $8 = 0;
 var $80 = 0, $81 = 0, $82 = 0, $83 = 0, $84 = 0, $85 = 0, $86 = 0, $87 = 0, $88 = 0, $89 = 0, $9 = 0, $90 = 0, $91 = 0, $92 = 0, $93 = 0, $94 = 0, $95 = 0, $96 = 0, $97 = 0, $98 = 0;
 var $99 = 0, $cond421 = 0, $cond422 = 0, $not$ = 0, $not$405 = 0, $not$437 = 0, label = 0, sp = 0;
 sp = STACKTOP;
 $1 = ($0|0)==(0|0);
 if ($1) {
  return;
 }
 $2 = ((($0)) + -8|0);
 $3 = HEAP32[(16452)>>2]|0;
 $4 = ($2>>>0)<($3>>>0);
 if ($4) {
  _abort();
  // unreachable;
 }
 $5 = ((($0)) + -4|0);
 $6 = HEAP32[$5>>2]|0;
 $7 = $6 & 3;
 $8 = ($7|0)==(1);
 if ($8) {
  _abort();
  // unreachable;
 }
 $9 = $6 & -8;
 $10 = (($2) + ($9)|0);
 $11 = $6 & 1;
 $12 = ($11|0)==(0);
 L10: do {
  if ($12) {
   $13 = HEAP32[$2>>2]|0;
   $14 = ($7|0)==(0);
   if ($14) {
    return;
   }
   $15 = (0 - ($13))|0;
   $16 = (($2) + ($15)|0);
   $17 = (($13) + ($9))|0;
   $18 = ($16>>>0)<($3>>>0);
   if ($18) {
    _abort();
    // unreachable;
   }
   $19 = HEAP32[(16456)>>2]|0;
   $20 = ($16|0)==($19|0);
   if ($20) {
    $104 = ((($10)) + 4|0);
    $105 = HEAP32[$104>>2]|0;
    $106 = $105 & 3;
    $107 = ($106|0)==(3);
    if (!($107)) {
     $$1 = $16;$$1382 = $17;$113 = $16;
     break;
    }
    $108 = (($16) + ($17)|0);
    $109 = ((($16)) + 4|0);
    $110 = $17 | 1;
    $111 = $105 & -2;
    HEAP32[(16444)>>2] = $17;
    HEAP32[$104>>2] = $111;
    HEAP32[$109>>2] = $110;
    HEAP32[$108>>2] = $17;
    return;
   }
   $21 = $13 >>> 3;
   $22 = ($13>>>0)<(256);
   if ($22) {
    $23 = ((($16)) + 8|0);
    $24 = HEAP32[$23>>2]|0;
    $25 = ((($16)) + 12|0);
    $26 = HEAP32[$25>>2]|0;
    $27 = $21 << 1;
    $28 = (16476 + ($27<<2)|0);
    $29 = ($24|0)==($28|0);
    if (!($29)) {
     $30 = ($24>>>0)<($3>>>0);
     if ($30) {
      _abort();
      // unreachable;
     }
     $31 = ((($24)) + 12|0);
     $32 = HEAP32[$31>>2]|0;
     $33 = ($32|0)==($16|0);
     if (!($33)) {
      _abort();
      // unreachable;
     }
    }
    $34 = ($26|0)==($24|0);
    if ($34) {
     $35 = 1 << $21;
     $36 = $35 ^ -1;
     $37 = HEAP32[4109]|0;
     $38 = $37 & $36;
     HEAP32[4109] = $38;
     $$1 = $16;$$1382 = $17;$113 = $16;
     break;
    }
    $39 = ($26|0)==($28|0);
    if ($39) {
     $$pre444 = ((($26)) + 8|0);
     $$pre$phi445Z2D = $$pre444;
    } else {
     $40 = ($26>>>0)<($3>>>0);
     if ($40) {
      _abort();
      // unreachable;
     }
     $41 = ((($26)) + 8|0);
     $42 = HEAP32[$41>>2]|0;
     $43 = ($42|0)==($16|0);
     if ($43) {
      $$pre$phi445Z2D = $41;
     } else {
      _abort();
      // unreachable;
     }
    }
    $44 = ((($24)) + 12|0);
    HEAP32[$44>>2] = $26;
    HEAP32[$$pre$phi445Z2D>>2] = $24;
    $$1 = $16;$$1382 = $17;$113 = $16;
    break;
   }
   $45 = ((($16)) + 24|0);
   $46 = HEAP32[$45>>2]|0;
   $47 = ((($16)) + 12|0);
   $48 = HEAP32[$47>>2]|0;
   $49 = ($48|0)==($16|0);
   do {
    if ($49) {
     $59 = ((($16)) + 16|0);
     $60 = ((($59)) + 4|0);
     $61 = HEAP32[$60>>2]|0;
     $62 = ($61|0)==(0|0);
     if ($62) {
      $63 = HEAP32[$59>>2]|0;
      $64 = ($63|0)==(0|0);
      if ($64) {
       $$3 = 0;
       break;
      } else {
       $$1387 = $63;$$1390 = $59;
      }
     } else {
      $$1387 = $61;$$1390 = $60;
     }
     while(1) {
      $65 = ((($$1387)) + 20|0);
      $66 = HEAP32[$65>>2]|0;
      $67 = ($66|0)==(0|0);
      if (!($67)) {
       $$1387 = $66;$$1390 = $65;
       continue;
      }
      $68 = ((($$1387)) + 16|0);
      $69 = HEAP32[$68>>2]|0;
      $70 = ($69|0)==(0|0);
      if ($70) {
       break;
      } else {
       $$1387 = $69;$$1390 = $68;
      }
     }
     $71 = ($$1390>>>0)<($3>>>0);
     if ($71) {
      _abort();
      // unreachable;
     } else {
      HEAP32[$$1390>>2] = 0;
      $$3 = $$1387;
      break;
     }
    } else {
     $50 = ((($16)) + 8|0);
     $51 = HEAP32[$50>>2]|0;
     $52 = ($51>>>0)<($3>>>0);
     if ($52) {
      _abort();
      // unreachable;
     }
     $53 = ((($51)) + 12|0);
     $54 = HEAP32[$53>>2]|0;
     $55 = ($54|0)==($16|0);
     if (!($55)) {
      _abort();
      // unreachable;
     }
     $56 = ((($48)) + 8|0);
     $57 = HEAP32[$56>>2]|0;
     $58 = ($57|0)==($16|0);
     if ($58) {
      HEAP32[$53>>2] = $48;
      HEAP32[$56>>2] = $51;
      $$3 = $48;
      break;
     } else {
      _abort();
      // unreachable;
     }
    }
   } while(0);
   $72 = ($46|0)==(0|0);
   if ($72) {
    $$1 = $16;$$1382 = $17;$113 = $16;
   } else {
    $73 = ((($16)) + 28|0);
    $74 = HEAP32[$73>>2]|0;
    $75 = (16740 + ($74<<2)|0);
    $76 = HEAP32[$75>>2]|0;
    $77 = ($16|0)==($76|0);
    do {
     if ($77) {
      HEAP32[$75>>2] = $$3;
      $cond421 = ($$3|0)==(0|0);
      if ($cond421) {
       $78 = 1 << $74;
       $79 = $78 ^ -1;
       $80 = HEAP32[(16440)>>2]|0;
       $81 = $80 & $79;
       HEAP32[(16440)>>2] = $81;
       $$1 = $16;$$1382 = $17;$113 = $16;
       break L10;
      }
     } else {
      $82 = HEAP32[(16452)>>2]|0;
      $83 = ($46>>>0)<($82>>>0);
      if ($83) {
       _abort();
       // unreachable;
      } else {
       $84 = ((($46)) + 16|0);
       $85 = HEAP32[$84>>2]|0;
       $not$405 = ($85|0)!=($16|0);
       $$sink3 = $not$405&1;
       $86 = (((($46)) + 16|0) + ($$sink3<<2)|0);
       HEAP32[$86>>2] = $$3;
       $87 = ($$3|0)==(0|0);
       if ($87) {
        $$1 = $16;$$1382 = $17;$113 = $16;
        break L10;
       } else {
        break;
       }
      }
     }
    } while(0);
    $88 = HEAP32[(16452)>>2]|0;
    $89 = ($$3>>>0)<($88>>>0);
    if ($89) {
     _abort();
     // unreachable;
    }
    $90 = ((($$3)) + 24|0);
    HEAP32[$90>>2] = $46;
    $91 = ((($16)) + 16|0);
    $92 = HEAP32[$91>>2]|0;
    $93 = ($92|0)==(0|0);
    do {
     if (!($93)) {
      $94 = ($92>>>0)<($88>>>0);
      if ($94) {
       _abort();
       // unreachable;
      } else {
       $95 = ((($$3)) + 16|0);
       HEAP32[$95>>2] = $92;
       $96 = ((($92)) + 24|0);
       HEAP32[$96>>2] = $$3;
       break;
      }
     }
    } while(0);
    $97 = ((($91)) + 4|0);
    $98 = HEAP32[$97>>2]|0;
    $99 = ($98|0)==(0|0);
    if ($99) {
     $$1 = $16;$$1382 = $17;$113 = $16;
    } else {
     $100 = HEAP32[(16452)>>2]|0;
     $101 = ($98>>>0)<($100>>>0);
     if ($101) {
      _abort();
      // unreachable;
     } else {
      $102 = ((($$3)) + 20|0);
      HEAP32[$102>>2] = $98;
      $103 = ((($98)) + 24|0);
      HEAP32[$103>>2] = $$3;
      $$1 = $16;$$1382 = $17;$113 = $16;
      break;
     }
    }
   }
  } else {
   $$1 = $2;$$1382 = $9;$113 = $2;
  }
 } while(0);
 $112 = ($113>>>0)<($10>>>0);
 if (!($112)) {
  _abort();
  // unreachable;
 }
 $114 = ((($10)) + 4|0);
 $115 = HEAP32[$114>>2]|0;
 $116 = $115 & 1;
 $117 = ($116|0)==(0);
 if ($117) {
  _abort();
  // unreachable;
 }
 $118 = $115 & 2;
 $119 = ($118|0)==(0);
 if ($119) {
  $120 = HEAP32[(16460)>>2]|0;
  $121 = ($10|0)==($120|0);
  $122 = HEAP32[(16456)>>2]|0;
  if ($121) {
   $123 = HEAP32[(16448)>>2]|0;
   $124 = (($123) + ($$1382))|0;
   HEAP32[(16448)>>2] = $124;
   HEAP32[(16460)>>2] = $$1;
   $125 = $124 | 1;
   $126 = ((($$1)) + 4|0);
   HEAP32[$126>>2] = $125;
   $127 = ($$1|0)==($122|0);
   if (!($127)) {
    return;
   }
   HEAP32[(16456)>>2] = 0;
   HEAP32[(16444)>>2] = 0;
   return;
  }
  $128 = ($10|0)==($122|0);
  if ($128) {
   $129 = HEAP32[(16444)>>2]|0;
   $130 = (($129) + ($$1382))|0;
   HEAP32[(16444)>>2] = $130;
   HEAP32[(16456)>>2] = $113;
   $131 = $130 | 1;
   $132 = ((($$1)) + 4|0);
   HEAP32[$132>>2] = $131;
   $133 = (($113) + ($130)|0);
   HEAP32[$133>>2] = $130;
   return;
  }
  $134 = $115 & -8;
  $135 = (($134) + ($$1382))|0;
  $136 = $115 >>> 3;
  $137 = ($115>>>0)<(256);
  L108: do {
   if ($137) {
    $138 = ((($10)) + 8|0);
    $139 = HEAP32[$138>>2]|0;
    $140 = ((($10)) + 12|0);
    $141 = HEAP32[$140>>2]|0;
    $142 = $136 << 1;
    $143 = (16476 + ($142<<2)|0);
    $144 = ($139|0)==($143|0);
    if (!($144)) {
     $145 = HEAP32[(16452)>>2]|0;
     $146 = ($139>>>0)<($145>>>0);
     if ($146) {
      _abort();
      // unreachable;
     }
     $147 = ((($139)) + 12|0);
     $148 = HEAP32[$147>>2]|0;
     $149 = ($148|0)==($10|0);
     if (!($149)) {
      _abort();
      // unreachable;
     }
    }
    $150 = ($141|0)==($139|0);
    if ($150) {
     $151 = 1 << $136;
     $152 = $151 ^ -1;
     $153 = HEAP32[4109]|0;
     $154 = $153 & $152;
     HEAP32[4109] = $154;
     break;
    }
    $155 = ($141|0)==($143|0);
    if ($155) {
     $$pre442 = ((($141)) + 8|0);
     $$pre$phi443Z2D = $$pre442;
    } else {
     $156 = HEAP32[(16452)>>2]|0;
     $157 = ($141>>>0)<($156>>>0);
     if ($157) {
      _abort();
      // unreachable;
     }
     $158 = ((($141)) + 8|0);
     $159 = HEAP32[$158>>2]|0;
     $160 = ($159|0)==($10|0);
     if ($160) {
      $$pre$phi443Z2D = $158;
     } else {
      _abort();
      // unreachable;
     }
    }
    $161 = ((($139)) + 12|0);
    HEAP32[$161>>2] = $141;
    HEAP32[$$pre$phi443Z2D>>2] = $139;
   } else {
    $162 = ((($10)) + 24|0);
    $163 = HEAP32[$162>>2]|0;
    $164 = ((($10)) + 12|0);
    $165 = HEAP32[$164>>2]|0;
    $166 = ($165|0)==($10|0);
    do {
     if ($166) {
      $177 = ((($10)) + 16|0);
      $178 = ((($177)) + 4|0);
      $179 = HEAP32[$178>>2]|0;
      $180 = ($179|0)==(0|0);
      if ($180) {
       $181 = HEAP32[$177>>2]|0;
       $182 = ($181|0)==(0|0);
       if ($182) {
        $$3400 = 0;
        break;
       } else {
        $$1398 = $181;$$1402 = $177;
       }
      } else {
       $$1398 = $179;$$1402 = $178;
      }
      while(1) {
       $183 = ((($$1398)) + 20|0);
       $184 = HEAP32[$183>>2]|0;
       $185 = ($184|0)==(0|0);
       if (!($185)) {
        $$1398 = $184;$$1402 = $183;
        continue;
       }
       $186 = ((($$1398)) + 16|0);
       $187 = HEAP32[$186>>2]|0;
       $188 = ($187|0)==(0|0);
       if ($188) {
        break;
       } else {
        $$1398 = $187;$$1402 = $186;
       }
      }
      $189 = HEAP32[(16452)>>2]|0;
      $190 = ($$1402>>>0)<($189>>>0);
      if ($190) {
       _abort();
       // unreachable;
      } else {
       HEAP32[$$1402>>2] = 0;
       $$3400 = $$1398;
       break;
      }
     } else {
      $167 = ((($10)) + 8|0);
      $168 = HEAP32[$167>>2]|0;
      $169 = HEAP32[(16452)>>2]|0;
      $170 = ($168>>>0)<($169>>>0);
      if ($170) {
       _abort();
       // unreachable;
      }
      $171 = ((($168)) + 12|0);
      $172 = HEAP32[$171>>2]|0;
      $173 = ($172|0)==($10|0);
      if (!($173)) {
       _abort();
       // unreachable;
      }
      $174 = ((($165)) + 8|0);
      $175 = HEAP32[$174>>2]|0;
      $176 = ($175|0)==($10|0);
      if ($176) {
       HEAP32[$171>>2] = $165;
       HEAP32[$174>>2] = $168;
       $$3400 = $165;
       break;
      } else {
       _abort();
       // unreachable;
      }
     }
    } while(0);
    $191 = ($163|0)==(0|0);
    if (!($191)) {
     $192 = ((($10)) + 28|0);
     $193 = HEAP32[$192>>2]|0;
     $194 = (16740 + ($193<<2)|0);
     $195 = HEAP32[$194>>2]|0;
     $196 = ($10|0)==($195|0);
     do {
      if ($196) {
       HEAP32[$194>>2] = $$3400;
       $cond422 = ($$3400|0)==(0|0);
       if ($cond422) {
        $197 = 1 << $193;
        $198 = $197 ^ -1;
        $199 = HEAP32[(16440)>>2]|0;
        $200 = $199 & $198;
        HEAP32[(16440)>>2] = $200;
        break L108;
       }
      } else {
       $201 = HEAP32[(16452)>>2]|0;
       $202 = ($163>>>0)<($201>>>0);
       if ($202) {
        _abort();
        // unreachable;
       } else {
        $203 = ((($163)) + 16|0);
        $204 = HEAP32[$203>>2]|0;
        $not$ = ($204|0)!=($10|0);
        $$sink5 = $not$&1;
        $205 = (((($163)) + 16|0) + ($$sink5<<2)|0);
        HEAP32[$205>>2] = $$3400;
        $206 = ($$3400|0)==(0|0);
        if ($206) {
         break L108;
        } else {
         break;
        }
       }
      }
     } while(0);
     $207 = HEAP32[(16452)>>2]|0;
     $208 = ($$3400>>>0)<($207>>>0);
     if ($208) {
      _abort();
      // unreachable;
     }
     $209 = ((($$3400)) + 24|0);
     HEAP32[$209>>2] = $163;
     $210 = ((($10)) + 16|0);
     $211 = HEAP32[$210>>2]|0;
     $212 = ($211|0)==(0|0);
     do {
      if (!($212)) {
       $213 = ($211>>>0)<($207>>>0);
       if ($213) {
        _abort();
        // unreachable;
       } else {
        $214 = ((($$3400)) + 16|0);
        HEAP32[$214>>2] = $211;
        $215 = ((($211)) + 24|0);
        HEAP32[$215>>2] = $$3400;
        break;
       }
      }
     } while(0);
     $216 = ((($210)) + 4|0);
     $217 = HEAP32[$216>>2]|0;
     $218 = ($217|0)==(0|0);
     if (!($218)) {
      $219 = HEAP32[(16452)>>2]|0;
      $220 = ($217>>>0)<($219>>>0);
      if ($220) {
       _abort();
       // unreachable;
      } else {
       $221 = ((($$3400)) + 20|0);
       HEAP32[$221>>2] = $217;
       $222 = ((($217)) + 24|0);
       HEAP32[$222>>2] = $$3400;
       break;
      }
     }
    }
   }
  } while(0);
  $223 = $135 | 1;
  $224 = ((($$1)) + 4|0);
  HEAP32[$224>>2] = $223;
  $225 = (($113) + ($135)|0);
  HEAP32[$225>>2] = $135;
  $226 = HEAP32[(16456)>>2]|0;
  $227 = ($$1|0)==($226|0);
  if ($227) {
   HEAP32[(16444)>>2] = $135;
   return;
  } else {
   $$2 = $135;
  }
 } else {
  $228 = $115 & -2;
  HEAP32[$114>>2] = $228;
  $229 = $$1382 | 1;
  $230 = ((($$1)) + 4|0);
  HEAP32[$230>>2] = $229;
  $231 = (($113) + ($$1382)|0);
  HEAP32[$231>>2] = $$1382;
  $$2 = $$1382;
 }
 $232 = $$2 >>> 3;
 $233 = ($$2>>>0)<(256);
 if ($233) {
  $234 = $232 << 1;
  $235 = (16476 + ($234<<2)|0);
  $236 = HEAP32[4109]|0;
  $237 = 1 << $232;
  $238 = $236 & $237;
  $239 = ($238|0)==(0);
  if ($239) {
   $240 = $236 | $237;
   HEAP32[4109] = $240;
   $$pre = ((($235)) + 8|0);
   $$0403 = $235;$$pre$phiZ2D = $$pre;
  } else {
   $241 = ((($235)) + 8|0);
   $242 = HEAP32[$241>>2]|0;
   $243 = HEAP32[(16452)>>2]|0;
   $244 = ($242>>>0)<($243>>>0);
   if ($244) {
    _abort();
    // unreachable;
   } else {
    $$0403 = $242;$$pre$phiZ2D = $241;
   }
  }
  HEAP32[$$pre$phiZ2D>>2] = $$1;
  $245 = ((($$0403)) + 12|0);
  HEAP32[$245>>2] = $$1;
  $246 = ((($$1)) + 8|0);
  HEAP32[$246>>2] = $$0403;
  $247 = ((($$1)) + 12|0);
  HEAP32[$247>>2] = $235;
  return;
 }
 $248 = $$2 >>> 8;
 $249 = ($248|0)==(0);
 if ($249) {
  $$0396 = 0;
 } else {
  $250 = ($$2>>>0)>(16777215);
  if ($250) {
   $$0396 = 31;
  } else {
   $251 = (($248) + 1048320)|0;
   $252 = $251 >>> 16;
   $253 = $252 & 8;
   $254 = $248 << $253;
   $255 = (($254) + 520192)|0;
   $256 = $255 >>> 16;
   $257 = $256 & 4;
   $258 = $257 | $253;
   $259 = $254 << $257;
   $260 = (($259) + 245760)|0;
   $261 = $260 >>> 16;
   $262 = $261 & 2;
   $263 = $258 | $262;
   $264 = (14 - ($263))|0;
   $265 = $259 << $262;
   $266 = $265 >>> 15;
   $267 = (($264) + ($266))|0;
   $268 = $267 << 1;
   $269 = (($267) + 7)|0;
   $270 = $$2 >>> $269;
   $271 = $270 & 1;
   $272 = $271 | $268;
   $$0396 = $272;
  }
 }
 $273 = (16740 + ($$0396<<2)|0);
 $274 = ((($$1)) + 28|0);
 HEAP32[$274>>2] = $$0396;
 $275 = ((($$1)) + 16|0);
 $276 = ((($$1)) + 20|0);
 HEAP32[$276>>2] = 0;
 HEAP32[$275>>2] = 0;
 $277 = HEAP32[(16440)>>2]|0;
 $278 = 1 << $$0396;
 $279 = $277 & $278;
 $280 = ($279|0)==(0);
 do {
  if ($280) {
   $281 = $277 | $278;
   HEAP32[(16440)>>2] = $281;
   HEAP32[$273>>2] = $$1;
   $282 = ((($$1)) + 24|0);
   HEAP32[$282>>2] = $273;
   $283 = ((($$1)) + 12|0);
   HEAP32[$283>>2] = $$1;
   $284 = ((($$1)) + 8|0);
   HEAP32[$284>>2] = $$1;
  } else {
   $285 = HEAP32[$273>>2]|0;
   $286 = ($$0396|0)==(31);
   $287 = $$0396 >>> 1;
   $288 = (25 - ($287))|0;
   $289 = $286 ? 0 : $288;
   $290 = $$2 << $289;
   $$0383 = $290;$$0384 = $285;
   while(1) {
    $291 = ((($$0384)) + 4|0);
    $292 = HEAP32[$291>>2]|0;
    $293 = $292 & -8;
    $294 = ($293|0)==($$2|0);
    if ($294) {
     label = 124;
     break;
    }
    $295 = $$0383 >>> 31;
    $296 = (((($$0384)) + 16|0) + ($295<<2)|0);
    $297 = $$0383 << 1;
    $298 = HEAP32[$296>>2]|0;
    $299 = ($298|0)==(0|0);
    if ($299) {
     label = 121;
     break;
    } else {
     $$0383 = $297;$$0384 = $298;
    }
   }
   if ((label|0) == 121) {
    $300 = HEAP32[(16452)>>2]|0;
    $301 = ($296>>>0)<($300>>>0);
    if ($301) {
     _abort();
     // unreachable;
    } else {
     HEAP32[$296>>2] = $$1;
     $302 = ((($$1)) + 24|0);
     HEAP32[$302>>2] = $$0384;
     $303 = ((($$1)) + 12|0);
     HEAP32[$303>>2] = $$1;
     $304 = ((($$1)) + 8|0);
     HEAP32[$304>>2] = $$1;
     break;
    }
   }
   else if ((label|0) == 124) {
    $305 = ((($$0384)) + 8|0);
    $306 = HEAP32[$305>>2]|0;
    $307 = HEAP32[(16452)>>2]|0;
    $308 = ($306>>>0)>=($307>>>0);
    $not$437 = ($$0384>>>0)>=($307>>>0);
    $309 = $308 & $not$437;
    if ($309) {
     $310 = ((($306)) + 12|0);
     HEAP32[$310>>2] = $$1;
     HEAP32[$305>>2] = $$1;
     $311 = ((($$1)) + 8|0);
     HEAP32[$311>>2] = $306;
     $312 = ((($$1)) + 12|0);
     HEAP32[$312>>2] = $$0384;
     $313 = ((($$1)) + 24|0);
     HEAP32[$313>>2] = 0;
     break;
    } else {
     _abort();
     // unreachable;
    }
   }
  }
 } while(0);
 $314 = HEAP32[(16468)>>2]|0;
 $315 = (($314) + -1)|0;
 HEAP32[(16468)>>2] = $315;
 $316 = ($315|0)==(0);
 if ($316) {
  $$0212$in$i = (16892);
 } else {
  return;
 }
 while(1) {
  $$0212$i = HEAP32[$$0212$in$i>>2]|0;
  $317 = ($$0212$i|0)==(0|0);
  $318 = ((($$0212$i)) + 8|0);
  if ($317) {
   break;
  } else {
   $$0212$in$i = $318;
  }
 }
 HEAP32[(16468)>>2] = -1;
 return;
}
function runPostSets() {
}
function _memset(ptr, value, num) {
    ptr = ptr|0; value = value|0; num = num|0;
    var end = 0, aligned_end = 0, block_aligned_end = 0, value4 = 0;
    end = (ptr + num)|0;

    value = value & 0xff;
    if ((num|0) >= 67 /* 64 bytes for an unrolled loop + 3 bytes for unaligned head*/) {
      while ((ptr&3) != 0) {
        HEAP8[((ptr)>>0)]=value;
        ptr = (ptr+1)|0;
      }

      aligned_end = (end & -4)|0;
      block_aligned_end = (aligned_end - 64)|0;
      value4 = value | (value << 8) | (value << 16) | (value << 24);

      while((ptr|0) <= (block_aligned_end|0)) {
        HEAP32[((ptr)>>2)]=value4;
        HEAP32[(((ptr)+(4))>>2)]=value4;
        HEAP32[(((ptr)+(8))>>2)]=value4;
        HEAP32[(((ptr)+(12))>>2)]=value4;
        HEAP32[(((ptr)+(16))>>2)]=value4;
        HEAP32[(((ptr)+(20))>>2)]=value4;
        HEAP32[(((ptr)+(24))>>2)]=value4;
        HEAP32[(((ptr)+(28))>>2)]=value4;
        HEAP32[(((ptr)+(32))>>2)]=value4;
        HEAP32[(((ptr)+(36))>>2)]=value4;
        HEAP32[(((ptr)+(40))>>2)]=value4;
        HEAP32[(((ptr)+(44))>>2)]=value4;
        HEAP32[(((ptr)+(48))>>2)]=value4;
        HEAP32[(((ptr)+(52))>>2)]=value4;
        HEAP32[(((ptr)+(56))>>2)]=value4;
        HEAP32[(((ptr)+(60))>>2)]=value4;
        ptr = (ptr + 64)|0;
      }

      while ((ptr|0) < (aligned_end|0) ) {
        HEAP32[((ptr)>>2)]=value4;
        ptr = (ptr+4)|0;
      }
    }
    // The remaining bytes.
    while ((ptr|0) < (end|0)) {
      HEAP8[((ptr)>>0)]=value;
      ptr = (ptr+1)|0;
    }
    return (end-num)|0;
}
function _i64Subtract(a, b, c, d) {
    a = a|0; b = b|0; c = c|0; d = d|0;
    var l = 0, h = 0;
    l = (a - c)>>>0;
    h = (b - d)>>>0;
    h = (b - d - (((c>>>0) > (a>>>0))|0))>>>0; // Borrow one from high word to low word on underflow.
    return ((tempRet0 = h,l|0)|0);
}
function _i64Add(a, b, c, d) {
    /*
      x = a + b*2^32
      y = c + d*2^32
      result = l + h*2^32
    */
    a = a|0; b = b|0; c = c|0; d = d|0;
    var l = 0, h = 0;
    l = (a + c)>>>0;
    h = (b + d + (((l>>>0) < (a>>>0))|0))>>>0; // Add carry from low word to high word on overflow.
    return ((tempRet0 = h,l|0)|0);
}
function _memcpy(dest, src, num) {
    dest = dest|0; src = src|0; num = num|0;
    var ret = 0;
    var aligned_dest_end = 0;
    var block_aligned_dest_end = 0;
    var dest_end = 0;
    // Test against a benchmarked cutoff limit for when HEAPU8.set() becomes faster to use.
    if ((num|0) >=
      8192
    ) {
      return _emscripten_memcpy_big(dest|0, src|0, num|0)|0;
    }

    ret = dest|0;
    dest_end = (dest + num)|0;
    if ((dest&3) == (src&3)) {
      // The initial unaligned < 4-byte front.
      while (dest & 3) {
        if ((num|0) == 0) return ret|0;
        HEAP8[((dest)>>0)]=((HEAP8[((src)>>0)])|0);
        dest = (dest+1)|0;
        src = (src+1)|0;
        num = (num-1)|0;
      }
      aligned_dest_end = (dest_end & -4)|0;
      block_aligned_dest_end = (aligned_dest_end - 64)|0;
      while ((dest|0) <= (block_aligned_dest_end|0) ) {
        HEAP32[((dest)>>2)]=((HEAP32[((src)>>2)])|0);
        HEAP32[(((dest)+(4))>>2)]=((HEAP32[(((src)+(4))>>2)])|0);
        HEAP32[(((dest)+(8))>>2)]=((HEAP32[(((src)+(8))>>2)])|0);
        HEAP32[(((dest)+(12))>>2)]=((HEAP32[(((src)+(12))>>2)])|0);
        HEAP32[(((dest)+(16))>>2)]=((HEAP32[(((src)+(16))>>2)])|0);
        HEAP32[(((dest)+(20))>>2)]=((HEAP32[(((src)+(20))>>2)])|0);
        HEAP32[(((dest)+(24))>>2)]=((HEAP32[(((src)+(24))>>2)])|0);
        HEAP32[(((dest)+(28))>>2)]=((HEAP32[(((src)+(28))>>2)])|0);
        HEAP32[(((dest)+(32))>>2)]=((HEAP32[(((src)+(32))>>2)])|0);
        HEAP32[(((dest)+(36))>>2)]=((HEAP32[(((src)+(36))>>2)])|0);
        HEAP32[(((dest)+(40))>>2)]=((HEAP32[(((src)+(40))>>2)])|0);
        HEAP32[(((dest)+(44))>>2)]=((HEAP32[(((src)+(44))>>2)])|0);
        HEAP32[(((dest)+(48))>>2)]=((HEAP32[(((src)+(48))>>2)])|0);
        HEAP32[(((dest)+(52))>>2)]=((HEAP32[(((src)+(52))>>2)])|0);
        HEAP32[(((dest)+(56))>>2)]=((HEAP32[(((src)+(56))>>2)])|0);
        HEAP32[(((dest)+(60))>>2)]=((HEAP32[(((src)+(60))>>2)])|0);
        dest = (dest+64)|0;
        src = (src+64)|0;
      }
      while ((dest|0) < (aligned_dest_end|0) ) {
        HEAP32[((dest)>>2)]=((HEAP32[((src)>>2)])|0);
        dest = (dest+4)|0;
        src = (src+4)|0;
      }
    } else {
      // In the unaligned copy case, unroll a bit as well.
      aligned_dest_end = (dest_end - 4)|0;
      while ((dest|0) < (aligned_dest_end|0) ) {
        HEAP8[((dest)>>0)]=((HEAP8[((src)>>0)])|0);
        HEAP8[(((dest)+(1))>>0)]=((HEAP8[(((src)+(1))>>0)])|0);
        HEAP8[(((dest)+(2))>>0)]=((HEAP8[(((src)+(2))>>0)])|0);
        HEAP8[(((dest)+(3))>>0)]=((HEAP8[(((src)+(3))>>0)])|0);
        dest = (dest+4)|0;
        src = (src+4)|0;
      }
    }
    // The remaining unaligned < 4 byte tail.
    while ((dest|0) < (dest_end|0)) {
      HEAP8[((dest)>>0)]=((HEAP8[((src)>>0)])|0);
      dest = (dest+1)|0;
      src = (src+1)|0;
    }
    return ret|0;
}
function _memmove(dest, src, num) {
    dest = dest|0; src = src|0; num = num|0;
    var ret = 0;
    if (((src|0) < (dest|0)) & ((dest|0) < ((src + num)|0))) {
      // Unlikely case: Copy backwards in a safe manner
      ret = dest;
      src = (src + num)|0;
      dest = (dest + num)|0;
      while ((num|0) > 0) {
        dest = (dest - 1)|0;
        src = (src - 1)|0;
        num = (num - 1)|0;
        HEAP8[((dest)>>0)]=((HEAP8[((src)>>0)])|0);
      }
      dest = ret;
    } else {
      _memcpy(dest, src, num) | 0;
    }
    return dest | 0;
}
function _llvm_cttz_i32(x) {
    x = x|0;
    var ret = 0;
    ret = ((HEAP8[(((cttz_i8)+(x & 0xff))>>0)])|0);
    if ((ret|0) < 8) return ret|0;
    ret = ((HEAP8[(((cttz_i8)+((x >> 8)&0xff))>>0)])|0);
    if ((ret|0) < 8) return (ret + 8)|0;
    ret = ((HEAP8[(((cttz_i8)+((x >> 16)&0xff))>>0)])|0);
    if ((ret|0) < 8) return (ret + 16)|0;
    return (((HEAP8[(((cttz_i8)+(x >>> 24))>>0)])|0) + 24)|0;
}
function ___udivmoddi4($a$0, $a$1, $b$0, $b$1, $rem) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    $rem = $rem | 0;
    var $n_sroa_0_0_extract_trunc = 0, $n_sroa_1_4_extract_shift$0 = 0, $n_sroa_1_4_extract_trunc = 0, $d_sroa_0_0_extract_trunc = 0, $d_sroa_1_4_extract_shift$0 = 0, $d_sroa_1_4_extract_trunc = 0, $4 = 0, $17 = 0, $37 = 0, $49 = 0, $51 = 0, $57 = 0, $58 = 0, $66 = 0, $78 = 0, $86 = 0, $88 = 0, $89 = 0, $91 = 0, $92 = 0, $95 = 0, $105 = 0, $117 = 0, $119 = 0, $125 = 0, $126 = 0, $130 = 0, $q_sroa_1_1_ph = 0, $q_sroa_0_1_ph = 0, $r_sroa_1_1_ph = 0, $r_sroa_0_1_ph = 0, $sr_1_ph = 0, $d_sroa_0_0_insert_insert99$0 = 0, $d_sroa_0_0_insert_insert99$1 = 0, $137$0 = 0, $137$1 = 0, $carry_0203 = 0, $sr_1202 = 0, $r_sroa_0_1201 = 0, $r_sroa_1_1200 = 0, $q_sroa_0_1199 = 0, $q_sroa_1_1198 = 0, $147 = 0, $149 = 0, $r_sroa_0_0_insert_insert42$0 = 0, $r_sroa_0_0_insert_insert42$1 = 0, $150$1 = 0, $151$0 = 0, $152 = 0, $154$0 = 0, $r_sroa_0_0_extract_trunc = 0, $r_sroa_1_4_extract_trunc = 0, $155 = 0, $carry_0_lcssa$0 = 0, $carry_0_lcssa$1 = 0, $r_sroa_0_1_lcssa = 0, $r_sroa_1_1_lcssa = 0, $q_sroa_0_1_lcssa = 0, $q_sroa_1_1_lcssa = 0, $q_sroa_0_0_insert_ext75$0 = 0, $q_sroa_0_0_insert_ext75$1 = 0, $q_sroa_0_0_insert_insert77$1 = 0, $_0$0 = 0, $_0$1 = 0;
    $n_sroa_0_0_extract_trunc = $a$0;
    $n_sroa_1_4_extract_shift$0 = $a$1;
    $n_sroa_1_4_extract_trunc = $n_sroa_1_4_extract_shift$0;
    $d_sroa_0_0_extract_trunc = $b$0;
    $d_sroa_1_4_extract_shift$0 = $b$1;
    $d_sroa_1_4_extract_trunc = $d_sroa_1_4_extract_shift$0;
    if (($n_sroa_1_4_extract_trunc | 0) == 0) {
      $4 = ($rem | 0) != 0;
      if (($d_sroa_1_4_extract_trunc | 0) == 0) {
        if ($4) {
          HEAP32[$rem >> 2] = ($n_sroa_0_0_extract_trunc >>> 0) % ($d_sroa_0_0_extract_trunc >>> 0);
          HEAP32[$rem + 4 >> 2] = 0;
        }
        $_0$1 = 0;
        $_0$0 = ($n_sroa_0_0_extract_trunc >>> 0) / ($d_sroa_0_0_extract_trunc >>> 0) >>> 0;
        return (tempRet0 = $_0$1, $_0$0) | 0;
      } else {
        if (!$4) {
          $_0$1 = 0;
          $_0$0 = 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        HEAP32[$rem >> 2] = $a$0 & -1;
        HEAP32[$rem + 4 >> 2] = $a$1 & 0;
        $_0$1 = 0;
        $_0$0 = 0;
        return (tempRet0 = $_0$1, $_0$0) | 0;
      }
    }
    $17 = ($d_sroa_1_4_extract_trunc | 0) == 0;
    do {
      if (($d_sroa_0_0_extract_trunc | 0) == 0) {
        if ($17) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = ($n_sroa_1_4_extract_trunc >>> 0) % ($d_sroa_0_0_extract_trunc >>> 0);
            HEAP32[$rem + 4 >> 2] = 0;
          }
          $_0$1 = 0;
          $_0$0 = ($n_sroa_1_4_extract_trunc >>> 0) / ($d_sroa_0_0_extract_trunc >>> 0) >>> 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        if (($n_sroa_0_0_extract_trunc | 0) == 0) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = 0;
            HEAP32[$rem + 4 >> 2] = ($n_sroa_1_4_extract_trunc >>> 0) % ($d_sroa_1_4_extract_trunc >>> 0);
          }
          $_0$1 = 0;
          $_0$0 = ($n_sroa_1_4_extract_trunc >>> 0) / ($d_sroa_1_4_extract_trunc >>> 0) >>> 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        $37 = $d_sroa_1_4_extract_trunc - 1 | 0;
        if (($37 & $d_sroa_1_4_extract_trunc | 0) == 0) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = 0 | $a$0 & -1;
            HEAP32[$rem + 4 >> 2] = $37 & $n_sroa_1_4_extract_trunc | $a$1 & 0;
          }
          $_0$1 = 0;
          $_0$0 = $n_sroa_1_4_extract_trunc >>> ((_llvm_cttz_i32($d_sroa_1_4_extract_trunc | 0) | 0) >>> 0);
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        $49 = Math_clz32($d_sroa_1_4_extract_trunc | 0) | 0;
        $51 = $49 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
        if ($51 >>> 0 <= 30) {
          $57 = $51 + 1 | 0;
          $58 = 31 - $51 | 0;
          $sr_1_ph = $57;
          $r_sroa_0_1_ph = $n_sroa_1_4_extract_trunc << $58 | $n_sroa_0_0_extract_trunc >>> ($57 >>> 0);
          $r_sroa_1_1_ph = $n_sroa_1_4_extract_trunc >>> ($57 >>> 0);
          $q_sroa_0_1_ph = 0;
          $q_sroa_1_1_ph = $n_sroa_0_0_extract_trunc << $58;
          break;
        }
        if (($rem | 0) == 0) {
          $_0$1 = 0;
          $_0$0 = 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        HEAP32[$rem >> 2] = 0 | $a$0 & -1;
        HEAP32[$rem + 4 >> 2] = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
        $_0$1 = 0;
        $_0$0 = 0;
        return (tempRet0 = $_0$1, $_0$0) | 0;
      } else {
        if (!$17) {
          $117 = Math_clz32($d_sroa_1_4_extract_trunc | 0) | 0;
          $119 = $117 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
          if ($119 >>> 0 <= 31) {
            $125 = $119 + 1 | 0;
            $126 = 31 - $119 | 0;
            $130 = $119 - 31 >> 31;
            $sr_1_ph = $125;
            $r_sroa_0_1_ph = $n_sroa_0_0_extract_trunc >>> ($125 >>> 0) & $130 | $n_sroa_1_4_extract_trunc << $126;
            $r_sroa_1_1_ph = $n_sroa_1_4_extract_trunc >>> ($125 >>> 0) & $130;
            $q_sroa_0_1_ph = 0;
            $q_sroa_1_1_ph = $n_sroa_0_0_extract_trunc << $126;
            break;
          }
          if (($rem | 0) == 0) {
            $_0$1 = 0;
            $_0$0 = 0;
            return (tempRet0 = $_0$1, $_0$0) | 0;
          }
          HEAP32[$rem >> 2] = 0 | $a$0 & -1;
          HEAP32[$rem + 4 >> 2] = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
          $_0$1 = 0;
          $_0$0 = 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
        $66 = $d_sroa_0_0_extract_trunc - 1 | 0;
        if (($66 & $d_sroa_0_0_extract_trunc | 0) != 0) {
          $86 = (Math_clz32($d_sroa_0_0_extract_trunc | 0) | 0) + 33 | 0;
          $88 = $86 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
          $89 = 64 - $88 | 0;
          $91 = 32 - $88 | 0;
          $92 = $91 >> 31;
          $95 = $88 - 32 | 0;
          $105 = $95 >> 31;
          $sr_1_ph = $88;
          $r_sroa_0_1_ph = $91 - 1 >> 31 & $n_sroa_1_4_extract_trunc >>> ($95 >>> 0) | ($n_sroa_1_4_extract_trunc << $91 | $n_sroa_0_0_extract_trunc >>> ($88 >>> 0)) & $105;
          $r_sroa_1_1_ph = $105 & $n_sroa_1_4_extract_trunc >>> ($88 >>> 0);
          $q_sroa_0_1_ph = $n_sroa_0_0_extract_trunc << $89 & $92;
          $q_sroa_1_1_ph = ($n_sroa_1_4_extract_trunc << $89 | $n_sroa_0_0_extract_trunc >>> ($95 >>> 0)) & $92 | $n_sroa_0_0_extract_trunc << $91 & $88 - 33 >> 31;
          break;
        }
        if (($rem | 0) != 0) {
          HEAP32[$rem >> 2] = $66 & $n_sroa_0_0_extract_trunc;
          HEAP32[$rem + 4 >> 2] = 0;
        }
        if (($d_sroa_0_0_extract_trunc | 0) == 1) {
          $_0$1 = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
          $_0$0 = 0 | $a$0 & -1;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        } else {
          $78 = _llvm_cttz_i32($d_sroa_0_0_extract_trunc | 0) | 0;
          $_0$1 = 0 | $n_sroa_1_4_extract_trunc >>> ($78 >>> 0);
          $_0$0 = $n_sroa_1_4_extract_trunc << 32 - $78 | $n_sroa_0_0_extract_trunc >>> ($78 >>> 0) | 0;
          return (tempRet0 = $_0$1, $_0$0) | 0;
        }
      }
    } while (0);
    if (($sr_1_ph | 0) == 0) {
      $q_sroa_1_1_lcssa = $q_sroa_1_1_ph;
      $q_sroa_0_1_lcssa = $q_sroa_0_1_ph;
      $r_sroa_1_1_lcssa = $r_sroa_1_1_ph;
      $r_sroa_0_1_lcssa = $r_sroa_0_1_ph;
      $carry_0_lcssa$1 = 0;
      $carry_0_lcssa$0 = 0;
    } else {
      $d_sroa_0_0_insert_insert99$0 = 0 | $b$0 & -1;
      $d_sroa_0_0_insert_insert99$1 = $d_sroa_1_4_extract_shift$0 | $b$1 & 0;
      $137$0 = _i64Add($d_sroa_0_0_insert_insert99$0 | 0, $d_sroa_0_0_insert_insert99$1 | 0, -1, -1) | 0;
      $137$1 = tempRet0;
      $q_sroa_1_1198 = $q_sroa_1_1_ph;
      $q_sroa_0_1199 = $q_sroa_0_1_ph;
      $r_sroa_1_1200 = $r_sroa_1_1_ph;
      $r_sroa_0_1201 = $r_sroa_0_1_ph;
      $sr_1202 = $sr_1_ph;
      $carry_0203 = 0;
      while (1) {
        $147 = $q_sroa_0_1199 >>> 31 | $q_sroa_1_1198 << 1;
        $149 = $carry_0203 | $q_sroa_0_1199 << 1;
        $r_sroa_0_0_insert_insert42$0 = 0 | ($r_sroa_0_1201 << 1 | $q_sroa_1_1198 >>> 31);
        $r_sroa_0_0_insert_insert42$1 = $r_sroa_0_1201 >>> 31 | $r_sroa_1_1200 << 1 | 0;
        _i64Subtract($137$0 | 0, $137$1 | 0, $r_sroa_0_0_insert_insert42$0 | 0, $r_sroa_0_0_insert_insert42$1 | 0) | 0;
        $150$1 = tempRet0;
        $151$0 = $150$1 >> 31 | (($150$1 | 0) < 0 ? -1 : 0) << 1;
        $152 = $151$0 & 1;
        $154$0 = _i64Subtract($r_sroa_0_0_insert_insert42$0 | 0, $r_sroa_0_0_insert_insert42$1 | 0, $151$0 & $d_sroa_0_0_insert_insert99$0 | 0, ((($150$1 | 0) < 0 ? -1 : 0) >> 31 | (($150$1 | 0) < 0 ? -1 : 0) << 1) & $d_sroa_0_0_insert_insert99$1 | 0) | 0;
        $r_sroa_0_0_extract_trunc = $154$0;
        $r_sroa_1_4_extract_trunc = tempRet0;
        $155 = $sr_1202 - 1 | 0;
        if (($155 | 0) == 0) {
          break;
        } else {
          $q_sroa_1_1198 = $147;
          $q_sroa_0_1199 = $149;
          $r_sroa_1_1200 = $r_sroa_1_4_extract_trunc;
          $r_sroa_0_1201 = $r_sroa_0_0_extract_trunc;
          $sr_1202 = $155;
          $carry_0203 = $152;
        }
      }
      $q_sroa_1_1_lcssa = $147;
      $q_sroa_0_1_lcssa = $149;
      $r_sroa_1_1_lcssa = $r_sroa_1_4_extract_trunc;
      $r_sroa_0_1_lcssa = $r_sroa_0_0_extract_trunc;
      $carry_0_lcssa$1 = 0;
      $carry_0_lcssa$0 = $152;
    }
    $q_sroa_0_0_insert_ext75$0 = $q_sroa_0_1_lcssa;
    $q_sroa_0_0_insert_ext75$1 = 0;
    $q_sroa_0_0_insert_insert77$1 = $q_sroa_1_1_lcssa | $q_sroa_0_0_insert_ext75$1;
    if (($rem | 0) != 0) {
      HEAP32[$rem >> 2] = 0 | $r_sroa_0_1_lcssa;
      HEAP32[$rem + 4 >> 2] = $r_sroa_1_1_lcssa | 0;
    }
    $_0$1 = (0 | $q_sroa_0_0_insert_ext75$0) >>> 31 | $q_sroa_0_0_insert_insert77$1 << 1 | ($q_sroa_0_0_insert_ext75$1 << 1 | $q_sroa_0_0_insert_ext75$0 >>> 31) & 0 | $carry_0_lcssa$1;
    $_0$0 = ($q_sroa_0_0_insert_ext75$0 << 1 | 0 >>> 31) & -2 | $carry_0_lcssa$0;
    return (tempRet0 = $_0$1, $_0$0) | 0;
}
function ___uremdi3($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $rem = 0, __stackBase__ = 0;
    __stackBase__ = STACKTOP;
    STACKTOP = STACKTOP + 16 | 0;
    $rem = __stackBase__ | 0;
    ___udivmoddi4($a$0, $a$1, $b$0, $b$1, $rem) | 0;
    STACKTOP = __stackBase__;
    return (tempRet0 = HEAP32[$rem + 4 >> 2] | 0, HEAP32[$rem >> 2] | 0) | 0;
}
function ___udivdi3($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $1$0 = 0;
    $1$0 = ___udivmoddi4($a$0, $a$1, $b$0, $b$1, 0) | 0;
    return $1$0 | 0;
}
function _roundf(f) {
    f = +f;
    return f >= +0 ? +Math_floor(f + +0.5) : +Math_ceil(f - +0.5); // TODO: use fround?
}
function _bitshift64Lshr(low, high, bits) {
    low = low|0; high = high|0; bits = bits|0;
    var ander = 0;
    if ((bits|0) < 32) {
      ander = ((1 << bits) - 1)|0;
      tempRet0 = high >>> bits;
      return (low >>> bits) | ((high&ander) << (32 - bits));
    }
    tempRet0 = 0;
    return (high >>> (bits - 32))|0;
}
function _sbrk(increment) {
    increment = increment|0;
    var oldDynamicTop = 0;
    var oldDynamicTopOnChange = 0;
    var newDynamicTop = 0;
    var totalMemory = 0;
    increment = ((increment + 15) & -16)|0;
    oldDynamicTop = HEAP32[DYNAMICTOP_PTR>>2]|0;
    newDynamicTop = oldDynamicTop + increment | 0;

    if (((increment|0) > 0 & (newDynamicTop|0) < (oldDynamicTop|0)) // Detect and fail if we would wrap around signed 32-bit int.
      | (newDynamicTop|0) < 0) { // Also underflow, sbrk() should be able to be used to subtract.
      abortOnCannotGrowMemory()|0;
      ___setErrNo(12);
      return -1;
    }

    HEAP32[DYNAMICTOP_PTR>>2] = newDynamicTop;
    totalMemory = getTotalMemory()|0;
    if ((newDynamicTop|0) > (totalMemory|0)) {
      if ((enlargeMemory()|0) == 0) {
        ___setErrNo(12);
        HEAP32[DYNAMICTOP_PTR>>2] = oldDynamicTop;
        return -1;
      }
    }
    return oldDynamicTop|0;
}
function _bitshift64Shl(low, high, bits) {
    low = low|0; high = high|0; bits = bits|0;
    var ander = 0;
    if ((bits|0) < 32) {
      ander = ((1 << bits) - 1)|0;
      tempRet0 = (high << bits) | ((low&(ander << (32 - bits))) >>> (32 - bits));
      return low << bits;
    }
    tempRet0 = low << (bits - 32);
    return 0;
}
function _llvm_bswap_i32(x) {
    x = x|0;
    return (((x&0xff)<<24) | (((x>>8)&0xff)<<16) | (((x>>16)&0xff)<<8) | (x>>>24))|0;
}

  
function dynCall_viiiii(index,a1,a2,a3,a4,a5) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0; a5=a5|0;
  FUNCTION_TABLE_viiiii[index&7](a1|0,a2|0,a3|0,a4|0,a5|0);
}


function dynCall_vd(index,a1) {
  index = index|0;
  a1=+a1;
  FUNCTION_TABLE_vd[index&3](+a1);
}


function dynCall_vid(index,a1,a2) {
  index = index|0;
  a1=a1|0; a2=+a2;
  FUNCTION_TABLE_vid[index&3](a1|0,+a2);
}


function dynCall_vi(index,a1) {
  index = index|0;
  a1=a1|0;
  FUNCTION_TABLE_vi[index&31](a1|0);
}


function dynCall_vii(index,a1,a2) {
  index = index|0;
  a1=a1|0; a2=a2|0;
  FUNCTION_TABLE_vii[index&63](a1|0,a2|0);
}


function dynCall_ii(index,a1) {
  index = index|0;
  a1=a1|0;
  return FUNCTION_TABLE_ii[index&15](a1|0)|0;
}


function dynCall_viddd(index,a1,a2,a3,a4) {
  index = index|0;
  a1=a1|0; a2=+a2; a3=+a3; a4=+a4;
  FUNCTION_TABLE_viddd[index&3](a1|0,+a2,+a3,+a4);
}


function dynCall_vidd(index,a1,a2,a3) {
  index = index|0;
  a1=a1|0; a2=+a2; a3=+a3;
  FUNCTION_TABLE_vidd[index&7](a1|0,+a2,+a3);
}


function dynCall_iiii(index,a1,a2,a3) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0;
  return FUNCTION_TABLE_iiii[index&15](a1|0,a2|0,a3|0)|0;
}


function dynCall_viiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0; a5=a5|0; a6=a6|0; a7=a7|0; a8=a8|0;
  FUNCTION_TABLE_viiiiiiii[index&3](a1|0,a2|0,a3|0,a4|0,a5|0,a6|0,a7|0,a8|0);
}


function dynCall_viiiiii(index,a1,a2,a3,a4,a5,a6) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0; a5=a5|0; a6=a6|0;
  FUNCTION_TABLE_viiiiii[index&3](a1|0,a2|0,a3|0,a4|0,a5|0,a6|0);
}


function dynCall_viii(index,a1,a2,a3) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0;
  FUNCTION_TABLE_viii[index&31](a1|0,a2|0,a3|0);
}


function dynCall_vidddd(index,a1,a2,a3,a4,a5) {
  index = index|0;
  a1=a1|0; a2=+a2; a3=+a3; a4=+a4; a5=+a5;
  FUNCTION_TABLE_vidddd[index&3](a1|0,+a2,+a3,+a4,+a5);
}


function dynCall_vdi(index,a1,a2) {
  index = index|0;
  a1=+a1; a2=a2|0;
  FUNCTION_TABLE_vdi[index&1](+a1,a2|0);
}


function dynCall_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0; a5=a5|0; a6=a6|0; a7=a7|0;
  FUNCTION_TABLE_viiiiiii[index&3](a1|0,a2|0,a3|0,a4|0,a5|0,a6|0,a7|0);
}


function dynCall_viiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0; a5=a5|0; a6=a6|0; a7=a7|0; a8=a8|0; a9=a9|0;
  FUNCTION_TABLE_viiiiiiiii[index&3](a1|0,a2|0,a3|0,a4|0,a5|0,a6|0,a7|0,a8|0,a9|0);
}


function dynCall_iii(index,a1,a2) {
  index = index|0;
  a1=a1|0; a2=a2|0;
  return FUNCTION_TABLE_iii[index&3](a1|0,a2|0)|0;
}


function dynCall_i(index) {
  index = index|0;
  
  return FUNCTION_TABLE_i[index&3]()|0;
}


function dynCall_vdddddd(index,a1,a2,a3,a4,a5,a6) {
  index = index|0;
  a1=+a1; a2=+a2; a3=+a3; a4=+a4; a5=+a5; a6=+a6;
  FUNCTION_TABLE_vdddddd[index&1](+a1,+a2,+a3,+a4,+a5,+a6);
}


function dynCall_vdddd(index,a1,a2,a3,a4) {
  index = index|0;
  a1=+a1; a2=+a2; a3=+a3; a4=+a4;
  FUNCTION_TABLE_vdddd[index&3](+a1,+a2,+a3,+a4);
}


function dynCall_vdd(index,a1,a2) {
  index = index|0;
  a1=+a1; a2=+a2;
  FUNCTION_TABLE_vdd[index&3](+a1,+a2);
}


function dynCall_v(index) {
  index = index|0;
  
  FUNCTION_TABLE_v[index&7]();
}


function dynCall_viid(index,a1,a2,a3) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=+a3;
  FUNCTION_TABLE_viid[index&1](a1|0,a2|0,+a3);
}


function dynCall_viiii(index,a1,a2,a3,a4) {
  index = index|0;
  a1=a1|0; a2=a2|0; a3=a3|0; a4=a4|0;
  FUNCTION_TABLE_viiii[index&31](a1|0,a2|0,a3|0,a4|0);
}

function b0(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0; nullFunc_viiiii(0);
}
function _emscripten_glUniform4i__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0; _emscripten_glUniform4i(p0|0,p1|0,p2|0,p3|0,p4|0);
}
function _emscripten_glFramebufferTexture2D__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0; _emscripten_glFramebufferTexture2D(p0|0,p1|0,p2|0,p3|0,p4|0);
}
function _emscripten_glShaderBinary__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0; _emscripten_glShaderBinary(p0|0,p1|0,p2|0,p3|0,p4|0);
}
function _emscripten_glDrawElementsInstanced__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0; _emscripten_glDrawElementsInstanced(p0|0,p1|0,p2|0,p3|0,p4|0);
}
function b1(p0) {
 p0 = +p0; nullFunc_vd(1);
}
function _emscripten_glClearDepth__wrapper(p0) {
 p0 = +p0; _emscripten_glClearDepth(+p0);
}
function _emscripten_glClearDepthf__wrapper(p0) {
 p0 = +p0; _emscripten_glClearDepthf(+p0);
}
function _emscripten_glLineWidth__wrapper(p0) {
 p0 = +p0; _emscripten_glLineWidth(+p0);
}
function b2(p0,p1) {
 p0 = p0|0;p1 = +p1; nullFunc_vid(2);
}
function _emscripten_glUniform1f__wrapper(p0,p1) {
 p0 = p0|0;p1 = +p1; _emscripten_glUniform1f(p0|0,+p1);
}
function _emscripten_glVertexAttrib1f__wrapper(p0,p1) {
 p0 = p0|0;p1 = +p1; _emscripten_glVertexAttrib1f(p0|0,+p1);
}
function b3(p0) {
 p0 = p0|0; nullFunc_vi(3);
}
function _emscripten_glDeleteShader__wrapper(p0) {
 p0 = p0|0; _emscripten_glDeleteShader(p0|0);
}
function _emscripten_glCompileShader__wrapper(p0) {
 p0 = p0|0; _emscripten_glCompileShader(p0|0);
}
function _emscripten_glDeleteProgram__wrapper(p0) {
 p0 = p0|0; _emscripten_glDeleteProgram(p0|0);
}
function _emscripten_glLinkProgram__wrapper(p0) {
 p0 = p0|0; _emscripten_glLinkProgram(p0|0);
}
function _emscripten_glUseProgram__wrapper(p0) {
 p0 = p0|0; _emscripten_glUseProgram(p0|0);
}
function _emscripten_glValidateProgram__wrapper(p0) {
 p0 = p0|0; _emscripten_glValidateProgram(p0|0);
}
function _emscripten_glDeleteObjectARB__wrapper(p0) {
 p0 = p0|0; _emscripten_glDeleteObjectARB(p0|0);
}
function _emscripten_glEnableClientState__wrapper(p0) {
 p0 = p0|0; _emscripten_glEnableClientState(p0|0);
}
function _emscripten_glClientActiveTexture__wrapper(p0) {
 p0 = p0|0; _emscripten_glClientActiveTexture(p0|0);
}
function _emscripten_glBindVertexArray__wrapper(p0) {
 p0 = p0|0; _emscripten_glBindVertexArray(p0|0);
}
function _emscripten_glMatrixMode__wrapper(p0) {
 p0 = p0|0; _emscripten_glMatrixMode(p0|0);
}
function _emscripten_glLoadMatrixf__wrapper(p0) {
 p0 = p0|0; _emscripten_glLoadMatrixf(p0|0);
}
function _emscripten_glEnableVertexAttribArray__wrapper(p0) {
 p0 = p0|0; _emscripten_glEnableVertexAttribArray(p0|0);
}
function _emscripten_glDisableVertexAttribArray__wrapper(p0) {
 p0 = p0|0; _emscripten_glDisableVertexAttribArray(p0|0);
}
function _emscripten_glDepthFunc__wrapper(p0) {
 p0 = p0|0; _emscripten_glDepthFunc(p0|0);
}
function _emscripten_glEnable__wrapper(p0) {
 p0 = p0|0; _emscripten_glEnable(p0|0);
}
function _emscripten_glDisable__wrapper(p0) {
 p0 = p0|0; _emscripten_glDisable(p0|0);
}
function _emscripten_glFrontFace__wrapper(p0) {
 p0 = p0|0; _emscripten_glFrontFace(p0|0);
}
function _emscripten_glCullFace__wrapper(p0) {
 p0 = p0|0; _emscripten_glCullFace(p0|0);
}
function _emscripten_glClear__wrapper(p0) {
 p0 = p0|0; _emscripten_glClear(p0|0);
}
function _emscripten_glClearStencil__wrapper(p0) {
 p0 = p0|0; _emscripten_glClearStencil(p0|0);
}
function _emscripten_glDepthMask__wrapper(p0) {
 p0 = p0|0; _emscripten_glDepthMask(p0|0);
}
function _emscripten_glStencilMask__wrapper(p0) {
 p0 = p0|0; _emscripten_glStencilMask(p0|0);
}
function _emscripten_glGenerateMipmap__wrapper(p0) {
 p0 = p0|0; _emscripten_glGenerateMipmap(p0|0);
}
function _emscripten_glActiveTexture__wrapper(p0) {
 p0 = p0|0; _emscripten_glActiveTexture(p0|0);
}
function _emscripten_glBlendEquation__wrapper(p0) {
 p0 = p0|0; _emscripten_glBlendEquation(p0|0);
}
function b4(p0,p1) {
 p0 = p0|0;p1 = p1|0; nullFunc_vii(4);
}
function _emscripten_glPixelStorei__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glPixelStorei(p0|0,p1|0);
}
function _emscripten_glGetIntegerv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGetIntegerv(p0|0,p1|0);
}
function _emscripten_glGetFloatv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGetFloatv(p0|0,p1|0);
}
function _emscripten_glGetBooleanv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGetBooleanv(p0|0,p1|0);
}
function _emscripten_glGenTextures__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGenTextures(p0|0,p1|0);
}
function _emscripten_glDeleteTextures__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDeleteTextures(p0|0,p1|0);
}
function _emscripten_glBindTexture__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBindTexture(p0|0,p1|0);
}
function _emscripten_glGenBuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGenBuffers(p0|0,p1|0);
}
function _emscripten_glDeleteBuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDeleteBuffers(p0|0,p1|0);
}
function _emscripten_glGenRenderbuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGenRenderbuffers(p0|0,p1|0);
}
function _emscripten_glDeleteRenderbuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDeleteRenderbuffers(p0|0,p1|0);
}
function _emscripten_glBindRenderbuffer__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBindRenderbuffer(p0|0,p1|0);
}
function _emscripten_glUniform1i__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glUniform1i(p0|0,p1|0);
}
function _emscripten_glBindBuffer__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBindBuffer(p0|0,p1|0);
}
function _emscripten_glVertexAttrib1fv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glVertexAttrib1fv(p0|0,p1|0);
}
function _emscripten_glVertexAttrib2fv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glVertexAttrib2fv(p0|0,p1|0);
}
function _emscripten_glVertexAttrib3fv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glVertexAttrib3fv(p0|0,p1|0);
}
function _emscripten_glVertexAttrib4fv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glVertexAttrib4fv(p0|0,p1|0);
}
function _emscripten_glAttachShader__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glAttachShader(p0|0,p1|0);
}
function _emscripten_glDetachShader__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDetachShader(p0|0,p1|0);
}
function _emscripten_glBindFramebuffer__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBindFramebuffer(p0|0,p1|0);
}
function _emscripten_glGenFramebuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGenFramebuffers(p0|0,p1|0);
}
function _emscripten_glDeleteFramebuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDeleteFramebuffers(p0|0,p1|0);
}
function _emscripten_glBindProgramARB__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBindProgramARB(p0|0,p1|0);
}
function _emscripten_glGetPointerv__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGetPointerv(p0|0,p1|0);
}
function _emscripten_glGenVertexArrays__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glGenVertexArrays(p0|0,p1|0);
}
function _emscripten_glDeleteVertexArrays__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDeleteVertexArrays(p0|0,p1|0);
}
function _emscripten_glVertexAttribDivisor__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glVertexAttribDivisor(p0|0,p1|0);
}
function _emscripten_glBlendFunc__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBlendFunc(p0|0,p1|0);
}
function _emscripten_glBlendEquationSeparate__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glBlendEquationSeparate(p0|0,p1|0);
}
function _emscripten_glStencilMaskSeparate__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glStencilMaskSeparate(p0|0,p1|0);
}
function _emscripten_glHint__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glHint(p0|0,p1|0);
}
function _emscripten_glDrawBuffers__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; _emscripten_glDrawBuffers(p0|0,p1|0);
}
function b5(p0) {
 p0 = p0|0; nullFunc_ii(5);return 0;
}
function _emscripten_glGetString__wrapper(p0) {
 p0 = p0|0; return _emscripten_glGetString(p0|0)|0;
}
function _emscripten_glIsTexture__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsTexture(p0|0)|0;
}
function _emscripten_glIsBuffer__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsBuffer(p0|0)|0;
}
function _emscripten_glIsRenderbuffer__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsRenderbuffer(p0|0)|0;
}
function _emscripten_glCreateShader__wrapper(p0) {
 p0 = p0|0; return _emscripten_glCreateShader(p0|0)|0;
}
function _emscripten_glIsShader__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsShader(p0|0)|0;
}
function _emscripten_glIsProgram__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsProgram(p0|0)|0;
}
function _emscripten_glIsFramebuffer__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsFramebuffer(p0|0)|0;
}
function _emscripten_glCheckFramebufferStatus__wrapper(p0) {
 p0 = p0|0; return _emscripten_glCheckFramebufferStatus(p0|0)|0;
}
function _emscripten_glIsEnabled__wrapper(p0) {
 p0 = p0|0; return _emscripten_glIsEnabled(p0|0)|0;
}
function b6(p0,p1,p2,p3) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3; nullFunc_viddd(6);
}
function _emscripten_glUniform3f__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3; _emscripten_glUniform3f(p0|0,+p1,+p2,+p3);
}
function _emscripten_glVertexAttrib3f__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3; _emscripten_glVertexAttrib3f(p0|0,+p1,+p2,+p3);
}
function b7(p0,p1,p2) {
 p0 = p0|0;p1 = +p1;p2 = +p2; nullFunc_vidd(7);
}
function _emscripten_glUniform2f__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = +p1;p2 = +p2; _emscripten_glUniform2f(p0|0,+p1,+p2);
}
function _emscripten_glVertexAttrib2f__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = +p1;p2 = +p2; _emscripten_glVertexAttrib2f(p0|0,+p1,+p2);
}
function b8(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; nullFunc_iiii(8);return 0;
}
function b9(p0,p1,p2,p3,p4,p5,p6,p7) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0; nullFunc_viiiiiiii(9);
}
function _emscripten_glCompressedTexImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0; _emscripten_glCompressedTexImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0);
}
function _emscripten_glCopyTexImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0; _emscripten_glCopyTexImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0);
}
function _emscripten_glCopyTexSubImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0; _emscripten_glCopyTexSubImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0);
}
function b10(p0,p1,p2,p3,p4,p5) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0; nullFunc_viiiiii(10);
}
function _emscripten_glDrawRangeElements__wrapper(p0,p1,p2,p3,p4,p5) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0; _emscripten_glDrawRangeElements(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0);
}
function _emscripten_glVertexAttribPointer__wrapper(p0,p1,p2,p3,p4,p5) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0; _emscripten_glVertexAttribPointer(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0);
}
function b11(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; nullFunc_viii(11);
}
function _emscripten_glGetTexParameterfv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetTexParameterfv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetTexParameteriv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetTexParameteriv(p0|0,p1|0,p2|0);
}
function _emscripten_glTexParameterfv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glTexParameterfv(p0|0,p1|0,p2|0);
}
function _emscripten_glTexParameteriv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glTexParameteriv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetBufferParameteriv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetBufferParameteriv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetRenderbufferParameteriv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetRenderbufferParameteriv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetUniformfv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetUniformfv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetUniformiv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetUniformiv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetVertexAttribfv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetVertexAttribfv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetVertexAttribiv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetVertexAttribiv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetVertexAttribPointerv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetVertexAttribPointerv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform2i__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform2i(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform1iv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform1iv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform2iv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform2iv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform3iv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform3iv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform4iv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform4iv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform1fv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform1fv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform2fv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform2fv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform3fv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform3fv(p0|0,p1|0,p2|0);
}
function _emscripten_glUniform4fv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glUniform4fv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetShaderiv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetShaderiv(p0|0,p1|0,p2|0);
}
function _emscripten_glGetProgramiv__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetProgramiv(p0|0,p1|0,p2|0);
}
function _emscripten_glBindAttribLocation__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glBindAttribLocation(p0|0,p1|0,p2|0);
}
function _emscripten_glGetObjectParameterivARB__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glGetObjectParameterivARB(p0|0,p1|0,p2|0);
}
function _emscripten_glNormalPointer__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glNormalPointer(p0|0,p1|0,p2|0);
}
function _emscripten_glDrawArrays__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glDrawArrays(p0|0,p1|0,p2|0);
}
function _emscripten_glTexParameteri__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glTexParameteri(p0|0,p1|0,p2|0);
}
function _emscripten_glStencilFunc__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glStencilFunc(p0|0,p1|0,p2|0);
}
function _emscripten_glStencilOp__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0; _emscripten_glStencilOp(p0|0,p1|0,p2|0);
}
function b12(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3;p4 = +p4; nullFunc_vidddd(12);
}
function _emscripten_glUniform4f__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3;p4 = +p4; _emscripten_glUniform4f(p0|0,+p1,+p2,+p3,+p4);
}
function _emscripten_glVertexAttrib4f__wrapper(p0,p1,p2,p3,p4) {
 p0 = p0|0;p1 = +p1;p2 = +p2;p3 = +p3;p4 = +p4; _emscripten_glVertexAttrib4f(p0|0,+p1,+p2,+p3,+p4);
}
function b13(p0,p1) {
 p0 = +p0;p1 = p1|0; nullFunc_vdi(13);
}
function _emscripten_glSampleCoverage__wrapper(p0,p1) {
 p0 = +p0;p1 = p1|0; _emscripten_glSampleCoverage(+p0,p1|0);
}
function b14(p0,p1,p2,p3,p4,p5,p6) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0; nullFunc_viiiiiii(14);
}
function _emscripten_glReadPixels__wrapper(p0,p1,p2,p3,p4,p5,p6) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0; _emscripten_glReadPixels(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0);
}
function _emscripten_glGetActiveUniform__wrapper(p0,p1,p2,p3,p4,p5,p6) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0; _emscripten_glGetActiveUniform(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0);
}
function _emscripten_glGetActiveAttrib__wrapper(p0,p1,p2,p3,p4,p5,p6) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0; _emscripten_glGetActiveAttrib(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0);
}
function b15(p0,p1,p2,p3,p4,p5,p6,p7,p8) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0;p8 = p8|0; nullFunc_viiiiiiiii(15);
}
function _emscripten_glCompressedTexSubImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7,p8) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0;p8 = p8|0; _emscripten_glCompressedTexSubImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0,p8|0);
}
function _emscripten_glTexImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7,p8) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0;p8 = p8|0; _emscripten_glTexImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0,p8|0);
}
function _emscripten_glTexSubImage2D__wrapper(p0,p1,p2,p3,p4,p5,p6,p7,p8) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0;p4 = p4|0;p5 = p5|0;p6 = p6|0;p7 = p7|0;p8 = p8|0; _emscripten_glTexSubImage2D(p0|0,p1|0,p2|0,p3|0,p4|0,p5|0,p6|0,p7|0,p8|0);
}
function b16(p0,p1) {
 p0 = p0|0;p1 = p1|0; nullFunc_iii(16);return 0;
}
function _emscripten_glGetUniformLocation__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; return _emscripten_glGetUniformLocation(p0|0,p1|0)|0;
}
function _emscripten_glGetAttribLocation__wrapper(p0,p1) {
 p0 = p0|0;p1 = p1|0; return _emscripten_glGetAttribLocation(p0|0,p1|0)|0;
}
function b17() {
 ; nullFunc_i(17);return 0;
}
function _emscripten_glCreateProgram__wrapper() {
 ; return _emscripten_glCreateProgram()|0;
}
function _emscripten_glGetError__wrapper() {
 ; return _emscripten_glGetError()|0;
}
function b18(p0,p1,p2,p3,p4,p5) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3;p4 = +p4;p5 = +p5; nullFunc_vdddddd(18);
}
function _emscripten_glFrustum__wrapper(p0,p1,p2,p3,p4,p5) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3;p4 = +p4;p5 = +p5; _emscripten_glFrustum(+p0,+p1,+p2,+p3,+p4,+p5);
}
function b19(p0,p1,p2,p3) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3; nullFunc_vdddd(19);
}
function _emscripten_glRotatef__wrapper(p0,p1,p2,p3) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3; _emscripten_glRotatef(+p0,+p1,+p2,+p3);
}
function _emscripten_glClearColor__wrapper(p0,p1,p2,p3) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3; _emscripten_glClearColor(+p0,+p1,+p2,+p3);
}
function _emscripten_glBlendColor__wrapper(p0,p1,p2,p3) {
 p0 = +p0;p1 = +p1;p2 = +p2;p3 = +p3; _emscripten_glBlendColor(+p0,+p1,+p2,+p3);
}
function b20(p0,p1) {
 p0 = +p0;p1 = +p1; nullFunc_vdd(20);
}
function _emscripten_glDepthRange__wrapper(p0,p1) {
 p0 = +p0;p1 = +p1; _emscripten_glDepthRange(+p0,+p1);
}
function _emscripten_glDepthRangef__wrapper(p0,p1) {
 p0 = +p0;p1 = +p1; _emscripten_glDepthRangef(+p0,+p1);
}
function _emscripten_glPolygonOffset__wrapper(p0,p1) {
 p0 = +p0;p1 = +p1; _emscripten_glPolygonOffset(+p0,+p1);
}
function b21() {
 ; nullFunc_v(21);
}
function _emscripten_glLoadIdentity__wrapper() {
 ; _emscripten_glLoadIdentity();
}
function _emscripten_glReleaseShaderCompiler__wrapper() {
 ; _emscripten_glReleaseShaderCompiler();
}
function _emscripten_glFinish__wrapper() {
 ; _emscripten_glFinish();
}
function _emscripten_glFlush__wrapper() {
 ; _emscripten_glFlush();
}
function b22(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = +p2; nullFunc_viid(22);
}
function _emscripten_glTexParameterf__wrapper(p0,p1,p2) {
 p0 = p0|0;p1 = p1|0;p2 = +p2; _emscripten_glTexParameterf(p0|0,p1|0,+p2);
}
function b23(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; nullFunc_viiii(23);
}
function _emscripten_glBufferData__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glBufferData(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glBufferSubData__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glBufferSubData(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glUniform3i__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glUniform3i(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glUniformMatrix2fv__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glUniformMatrix2fv(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glUniformMatrix3fv__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glUniformMatrix3fv(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glUniformMatrix4fv__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glUniformMatrix4fv(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetAttachedShaders__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetAttachedShaders(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glShaderSource__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glShaderSource(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetShaderSource__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetShaderSource(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetShaderInfoLog__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetShaderInfoLog(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetShaderPrecisionFormat__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetShaderPrecisionFormat(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetProgramInfoLog__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetProgramInfoLog(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glFramebufferRenderbuffer__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glFramebufferRenderbuffer(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetFramebufferAttachmentParameteriv__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetFramebufferAttachmentParameteriv(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glGetInfoLogARB__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glGetInfoLogARB(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glVertexPointer__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glVertexPointer(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glTexCoordPointer__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glTexCoordPointer(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glColorPointer__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glColorPointer(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glDrawElements__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glDrawElements(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glDrawArraysInstanced__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glDrawArraysInstanced(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glViewport__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glViewport(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glScissor__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glScissor(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glColorMask__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glColorMask(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glRenderbufferStorage__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glRenderbufferStorage(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glBlendFuncSeparate__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glBlendFuncSeparate(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glStencilFuncSeparate__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glStencilFuncSeparate(p0|0,p1|0,p2|0,p3|0);
}
function _emscripten_glStencilOpSeparate__wrapper(p0,p1,p2,p3) {
 p0 = p0|0;p1 = p1|0;p2 = p2|0;p3 = p3|0; _emscripten_glStencilOpSeparate(p0|0,p1|0,p2|0,p3|0);
}

// EMSCRIPTEN_END_FUNCS
var FUNCTION_TABLE_viiiii = [b0,_KeyCallback,_emscripten_glUniform4i__wrapper,_emscripten_glFramebufferTexture2D__wrapper,_emscripten_glShaderBinary__wrapper,_emscripten_glDrawElementsInstanced__wrapper,b0,b0];
var FUNCTION_TABLE_vd = [b1,_emscripten_glClearDepth__wrapper,_emscripten_glClearDepthf__wrapper,_emscripten_glLineWidth__wrapper];
var FUNCTION_TABLE_vid = [b2,_emscripten_glUniform1f__wrapper,_emscripten_glVertexAttrib1f__wrapper,b2];
var FUNCTION_TABLE_vi = [b3,_emscripten_glDeleteShader__wrapper,_emscripten_glCompileShader__wrapper,_emscripten_glDeleteProgram__wrapper,_emscripten_glLinkProgram__wrapper,_emscripten_glUseProgram__wrapper,_emscripten_glValidateProgram__wrapper,_emscripten_glDeleteObjectARB__wrapper,_emscripten_glEnableClientState__wrapper,_emscripten_glClientActiveTexture__wrapper,_emscripten_glBindVertexArray__wrapper,_emscripten_glMatrixMode__wrapper,_emscripten_glLoadMatrixf__wrapper,_emscripten_glEnableVertexAttribArray__wrapper,_emscripten_glDisableVertexAttribArray__wrapper,_emscripten_glDepthFunc__wrapper,_emscripten_glEnable__wrapper,_emscripten_glDisable__wrapper,_emscripten_glFrontFace__wrapper,_emscripten_glCullFace__wrapper,_emscripten_glClear__wrapper,_emscripten_glClearStencil__wrapper,_emscripten_glDepthMask__wrapper,_emscripten_glStencilMask__wrapper,_emscripten_glGenerateMipmap__wrapper,_emscripten_glActiveTexture__wrapper,_emscripten_glBlendEquation__wrapper,b3,b3
,b3,b3,b3];
var FUNCTION_TABLE_vii = [b4,_ErrorCallback,_CursorEnterCallback,_CharCallback,_WindowIconifyCallback,_emscripten_glPixelStorei__wrapper,_emscripten_glGetIntegerv__wrapper,_emscripten_glGetFloatv__wrapper,_emscripten_glGetBooleanv__wrapper,_emscripten_glGenTextures__wrapper,_emscripten_glDeleteTextures__wrapper,_emscripten_glBindTexture__wrapper,_emscripten_glGenBuffers__wrapper,_emscripten_glDeleteBuffers__wrapper,_emscripten_glGenRenderbuffers__wrapper,_emscripten_glDeleteRenderbuffers__wrapper,_emscripten_glBindRenderbuffer__wrapper,_emscripten_glUniform1i__wrapper,_emscripten_glBindBuffer__wrapper,_emscripten_glVertexAttrib1fv__wrapper,_emscripten_glVertexAttrib2fv__wrapper,_emscripten_glVertexAttrib3fv__wrapper,_emscripten_glVertexAttrib4fv__wrapper,_emscripten_glAttachShader__wrapper,_emscripten_glDetachShader__wrapper,_emscripten_glBindFramebuffer__wrapper,_emscripten_glGenFramebuffers__wrapper,_emscripten_glDeleteFramebuffers__wrapper,_emscripten_glBindProgramARB__wrapper,_emscripten_glGetPointerv__wrapper,_emscripten_glGenVertexArrays__wrapper,_emscripten_glDeleteVertexArrays__wrapper,_emscripten_glVertexAttribDivisor__wrapper,_emscripten_glBlendFunc__wrapper,_emscripten_glBlendEquationSeparate__wrapper,_emscripten_glStencilMaskSeparate__wrapper,_emscripten_glHint__wrapper,_emscripten_glDrawBuffers__wrapper,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4,b4
,b4,b4,b4,b4,b4];
var FUNCTION_TABLE_ii = [b5,___stdio_close,_emscripten_glGetString__wrapper,_emscripten_glIsTexture__wrapper,_emscripten_glIsBuffer__wrapper,_emscripten_glIsRenderbuffer__wrapper,_emscripten_glCreateShader__wrapper,_emscripten_glIsShader__wrapper,_emscripten_glIsProgram__wrapper,_emscripten_glIsFramebuffer__wrapper,_emscripten_glCheckFramebufferStatus__wrapper,_emscripten_glIsEnabled__wrapper,b5,b5,b5,b5];
var FUNCTION_TABLE_viddd = [b6,_emscripten_glUniform3f__wrapper,_emscripten_glVertexAttrib3f__wrapper,b6];
var FUNCTION_TABLE_vidd = [b7,_MouseCursorPosCallback,_ScrollCallback,_emscripten_glUniform2f__wrapper,_emscripten_glVertexAttrib2f__wrapper,b7,b7,b7];
var FUNCTION_TABLE_iiii = [b8,___stdout_write,___stdio_seek,_sn_write,_EmscriptenFullscreenChangeCallback,_EmscriptenKeyboardCallback,_EmscriptenMouseCallback,_EmscriptenTouchCallback,_EmscriptenGamepadCallback,___stdio_write,b8,b8,b8,b8,b8,b8];
var FUNCTION_TABLE_viiiiiiii = [b9,_emscripten_glCompressedTexImage2D__wrapper,_emscripten_glCopyTexImage2D__wrapper,_emscripten_glCopyTexSubImage2D__wrapper];
var FUNCTION_TABLE_viiiiii = [b10,_emscripten_glDrawRangeElements__wrapper,_emscripten_glVertexAttribPointer__wrapper,b10];
var FUNCTION_TABLE_viii = [b11,_WindowSizeCallback,_emscripten_glGetTexParameterfv__wrapper,_emscripten_glGetTexParameteriv__wrapper,_emscripten_glTexParameterfv__wrapper,_emscripten_glTexParameteriv__wrapper,_emscripten_glGetBufferParameteriv__wrapper,_emscripten_glGetRenderbufferParameteriv__wrapper,_emscripten_glGetUniformfv__wrapper,_emscripten_glGetUniformiv__wrapper,_emscripten_glGetVertexAttribfv__wrapper,_emscripten_glGetVertexAttribiv__wrapper,_emscripten_glGetVertexAttribPointerv__wrapper,_emscripten_glUniform2i__wrapper,_emscripten_glUniform1iv__wrapper,_emscripten_glUniform2iv__wrapper,_emscripten_glUniform3iv__wrapper,_emscripten_glUniform4iv__wrapper,_emscripten_glUniform1fv__wrapper,_emscripten_glUniform2fv__wrapper,_emscripten_glUniform3fv__wrapper,_emscripten_glUniform4fv__wrapper,_emscripten_glGetShaderiv__wrapper,_emscripten_glGetProgramiv__wrapper,_emscripten_glBindAttribLocation__wrapper,_emscripten_glGetObjectParameterivARB__wrapper,_emscripten_glNormalPointer__wrapper,_emscripten_glDrawArrays__wrapper,_emscripten_glTexParameteri__wrapper,_emscripten_glStencilFunc__wrapper,_emscripten_glStencilOp__wrapper,b11];
var FUNCTION_TABLE_vidddd = [b12,_emscripten_glUniform4f__wrapper,_emscripten_glVertexAttrib4f__wrapper,b12];
var FUNCTION_TABLE_vdi = [b13,_emscripten_glSampleCoverage__wrapper];
var FUNCTION_TABLE_viiiiiii = [b14,_emscripten_glReadPixels__wrapper,_emscripten_glGetActiveUniform__wrapper,_emscripten_glGetActiveAttrib__wrapper];
var FUNCTION_TABLE_viiiiiiiii = [b15,_emscripten_glCompressedTexSubImage2D__wrapper,_emscripten_glTexImage2D__wrapper,_emscripten_glTexSubImage2D__wrapper];
var FUNCTION_TABLE_iii = [b16,_emscripten_glGetUniformLocation__wrapper,_emscripten_glGetAttribLocation__wrapper,b16];
var FUNCTION_TABLE_i = [b17,_emscripten_glCreateProgram__wrapper,_emscripten_glGetError__wrapper,b17];
var FUNCTION_TABLE_vdddddd = [b18,_emscripten_glFrustum__wrapper];
var FUNCTION_TABLE_vdddd = [b19,_emscripten_glRotatef__wrapper,_emscripten_glClearColor__wrapper,_emscripten_glBlendColor__wrapper];
var FUNCTION_TABLE_vdd = [b20,_emscripten_glDepthRange__wrapper,_emscripten_glDepthRangef__wrapper,_emscripten_glPolygonOffset__wrapper];
var FUNCTION_TABLE_v = [b21,_UpdateDrawFrame,_emscripten_glLoadIdentity__wrapper,_emscripten_glReleaseShaderCompiler__wrapper,_emscripten_glFinish__wrapper,_emscripten_glFlush__wrapper,b21,b21];
var FUNCTION_TABLE_viid = [b22,_emscripten_glTexParameterf__wrapper];
var FUNCTION_TABLE_viiii = [b23,_MouseButtonCallback,_emscripten_glBufferData__wrapper,_emscripten_glBufferSubData__wrapper,_emscripten_glUniform3i__wrapper,_emscripten_glUniformMatrix2fv__wrapper,_emscripten_glUniformMatrix3fv__wrapper,_emscripten_glUniformMatrix4fv__wrapper,_emscripten_glGetAttachedShaders__wrapper,_emscripten_glShaderSource__wrapper,_emscripten_glGetShaderSource__wrapper,_emscripten_glGetShaderInfoLog__wrapper,_emscripten_glGetShaderPrecisionFormat__wrapper,_emscripten_glGetProgramInfoLog__wrapper,_emscripten_glFramebufferRenderbuffer__wrapper,_emscripten_glGetFramebufferAttachmentParameteriv__wrapper,_emscripten_glGetInfoLogARB__wrapper,_emscripten_glVertexPointer__wrapper,_emscripten_glTexCoordPointer__wrapper,_emscripten_glColorPointer__wrapper,_emscripten_glDrawElements__wrapper,_emscripten_glDrawArraysInstanced__wrapper,_emscripten_glViewport__wrapper,_emscripten_glScissor__wrapper,_emscripten_glColorMask__wrapper,_emscripten_glRenderbufferStorage__wrapper,_emscripten_glBlendFuncSeparate__wrapper,_emscripten_glStencilFuncSeparate__wrapper,_emscripten_glStencilOpSeparate__wrapper,b23,b23,b23];

  return { _roundf: _roundf, _main: _main, _llvm_cttz_i32: _llvm_cttz_i32, _bitshift64Lshr: _bitshift64Lshr, _bitshift64Shl: _bitshift64Shl, _fflush: _fflush, _memset: _memset, _sbrk: _sbrk, _memcpy: _memcpy, ___errno_location: ___errno_location, ___uremdi3: ___uremdi3, _i64Subtract: _i64Subtract, ___udivmoddi4: ___udivmoddi4, _i64Add: _i64Add, _emscripten_get_global_libc: _emscripten_get_global_libc, _emscripten_GetProcAddress: _emscripten_GetProcAddress, ___udivdi3: ___udivdi3, _llvm_bswap_i32: _llvm_bswap_i32, _free: _free, _memmove: _memmove, _strstr: _strstr, _malloc: _malloc, runPostSets: runPostSets, stackAlloc: stackAlloc, stackSave: stackSave, stackRestore: stackRestore, establishStackSpace: establishStackSpace, setTempRet0: setTempRet0, getTempRet0: getTempRet0, setThrew: setThrew, stackAlloc: stackAlloc, stackSave: stackSave, stackRestore: stackRestore, establishStackSpace: establishStackSpace, setThrew: setThrew, setTempRet0: setTempRet0, getTempRet0: getTempRet0, dynCall_viiiii: dynCall_viiiii, dynCall_vd: dynCall_vd, dynCall_vid: dynCall_vid, dynCall_vi: dynCall_vi, dynCall_vii: dynCall_vii, dynCall_ii: dynCall_ii, dynCall_viddd: dynCall_viddd, dynCall_vidd: dynCall_vidd, dynCall_iiii: dynCall_iiii, dynCall_viiiiiiii: dynCall_viiiiiiii, dynCall_viiiiii: dynCall_viiiiii, dynCall_viii: dynCall_viii, dynCall_vidddd: dynCall_vidddd, dynCall_vdi: dynCall_vdi, dynCall_viiiiiii: dynCall_viiiiiii, dynCall_viiiiiiiii: dynCall_viiiiiiiii, dynCall_iii: dynCall_iii, dynCall_i: dynCall_i, dynCall_vdddddd: dynCall_vdddddd, dynCall_vdddd: dynCall_vdddd, dynCall_vdd: dynCall_vdd, dynCall_v: dynCall_v, dynCall_viid: dynCall_viid, dynCall_viiii: dynCall_viiii };
})
// EMSCRIPTEN_END_ASM
(Module.asmGlobalArg, Module.asmLibraryArg, buffer);

var real__roundf = asm["_roundf"]; asm["_roundf"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__roundf.apply(null, arguments);
};

var real__main = asm["_main"]; asm["_main"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__main.apply(null, arguments);
};

var real_stackSave = asm["stackSave"]; asm["stackSave"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_stackSave.apply(null, arguments);
};

var real_getTempRet0 = asm["getTempRet0"]; asm["getTempRet0"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_getTempRet0.apply(null, arguments);
};

var real_setThrew = asm["setThrew"]; asm["setThrew"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_setThrew.apply(null, arguments);
};

var real__bitshift64Lshr = asm["_bitshift64Lshr"]; asm["_bitshift64Lshr"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__bitshift64Lshr.apply(null, arguments);
};

var real__bitshift64Shl = asm["_bitshift64Shl"]; asm["_bitshift64Shl"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__bitshift64Shl.apply(null, arguments);
};

var real__fflush = asm["_fflush"]; asm["_fflush"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__fflush.apply(null, arguments);
};

var real__llvm_cttz_i32 = asm["_llvm_cttz_i32"]; asm["_llvm_cttz_i32"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__llvm_cttz_i32.apply(null, arguments);
};

var real__sbrk = asm["_sbrk"]; asm["_sbrk"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__sbrk.apply(null, arguments);
};

var real____errno_location = asm["___errno_location"]; asm["___errno_location"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real____errno_location.apply(null, arguments);
};

var real____uremdi3 = asm["___uremdi3"]; asm["___uremdi3"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real____uremdi3.apply(null, arguments);
};

var real_stackAlloc = asm["stackAlloc"]; asm["stackAlloc"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_stackAlloc.apply(null, arguments);
};

var real__i64Subtract = asm["_i64Subtract"]; asm["_i64Subtract"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__i64Subtract.apply(null, arguments);
};

var real____udivmoddi4 = asm["___udivmoddi4"]; asm["___udivmoddi4"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real____udivmoddi4.apply(null, arguments);
};

var real_setTempRet0 = asm["setTempRet0"]; asm["setTempRet0"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_setTempRet0.apply(null, arguments);
};

var real__i64Add = asm["_i64Add"]; asm["_i64Add"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__i64Add.apply(null, arguments);
};

var real__emscripten_get_global_libc = asm["_emscripten_get_global_libc"]; asm["_emscripten_get_global_libc"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__emscripten_get_global_libc.apply(null, arguments);
};

var real__emscripten_GetProcAddress = asm["_emscripten_GetProcAddress"]; asm["_emscripten_GetProcAddress"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__emscripten_GetProcAddress.apply(null, arguments);
};

var real____udivdi3 = asm["___udivdi3"]; asm["___udivdi3"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real____udivdi3.apply(null, arguments);
};

var real__llvm_bswap_i32 = asm["_llvm_bswap_i32"]; asm["_llvm_bswap_i32"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__llvm_bswap_i32.apply(null, arguments);
};

var real__free = asm["_free"]; asm["_free"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__free.apply(null, arguments);
};

var real_establishStackSpace = asm["establishStackSpace"]; asm["establishStackSpace"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_establishStackSpace.apply(null, arguments);
};

var real__memmove = asm["_memmove"]; asm["_memmove"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__memmove.apply(null, arguments);
};

var real__strstr = asm["_strstr"]; asm["_strstr"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__strstr.apply(null, arguments);
};

var real_stackRestore = asm["stackRestore"]; asm["stackRestore"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_stackRestore.apply(null, arguments);
};

var real__malloc = asm["_malloc"]; asm["_malloc"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real__malloc.apply(null, arguments);
};
var _roundf = Module["_roundf"] = asm["_roundf"];
var _main = Module["_main"] = asm["_main"];
var stackSave = Module["stackSave"] = asm["stackSave"];
var getTempRet0 = Module["getTempRet0"] = asm["getTempRet0"];
var _memset = Module["_memset"] = asm["_memset"];
var setThrew = Module["setThrew"] = asm["setThrew"];
var _bitshift64Lshr = Module["_bitshift64Lshr"] = asm["_bitshift64Lshr"];
var _bitshift64Shl = Module["_bitshift64Shl"] = asm["_bitshift64Shl"];
var _fflush = Module["_fflush"] = asm["_fflush"];
var _llvm_cttz_i32 = Module["_llvm_cttz_i32"] = asm["_llvm_cttz_i32"];
var _sbrk = Module["_sbrk"] = asm["_sbrk"];
var _memcpy = Module["_memcpy"] = asm["_memcpy"];
var ___errno_location = Module["___errno_location"] = asm["___errno_location"];
var ___uremdi3 = Module["___uremdi3"] = asm["___uremdi3"];
var stackAlloc = Module["stackAlloc"] = asm["stackAlloc"];
var _i64Subtract = Module["_i64Subtract"] = asm["_i64Subtract"];
var ___udivmoddi4 = Module["___udivmoddi4"] = asm["___udivmoddi4"];
var setTempRet0 = Module["setTempRet0"] = asm["setTempRet0"];
var _i64Add = Module["_i64Add"] = asm["_i64Add"];
var _emscripten_get_global_libc = Module["_emscripten_get_global_libc"] = asm["_emscripten_get_global_libc"];
var _emscripten_GetProcAddress = Module["_emscripten_GetProcAddress"] = asm["_emscripten_GetProcAddress"];
var ___udivdi3 = Module["___udivdi3"] = asm["___udivdi3"];
var _llvm_bswap_i32 = Module["_llvm_bswap_i32"] = asm["_llvm_bswap_i32"];
var _free = Module["_free"] = asm["_free"];
var runPostSets = Module["runPostSets"] = asm["runPostSets"];
var establishStackSpace = Module["establishStackSpace"] = asm["establishStackSpace"];
var _memmove = Module["_memmove"] = asm["_memmove"];
var _strstr = Module["_strstr"] = asm["_strstr"];
var stackRestore = Module["stackRestore"] = asm["stackRestore"];
var _malloc = Module["_malloc"] = asm["_malloc"];
var dynCall_viiiii = Module["dynCall_viiiii"] = asm["dynCall_viiiii"];
var dynCall_vd = Module["dynCall_vd"] = asm["dynCall_vd"];
var dynCall_vid = Module["dynCall_vid"] = asm["dynCall_vid"];
var dynCall_vi = Module["dynCall_vi"] = asm["dynCall_vi"];
var dynCall_vii = Module["dynCall_vii"] = asm["dynCall_vii"];
var dynCall_ii = Module["dynCall_ii"] = asm["dynCall_ii"];
var dynCall_viddd = Module["dynCall_viddd"] = asm["dynCall_viddd"];
var dynCall_vidd = Module["dynCall_vidd"] = asm["dynCall_vidd"];
var dynCall_iiii = Module["dynCall_iiii"] = asm["dynCall_iiii"];
var dynCall_viiiiiiii = Module["dynCall_viiiiiiii"] = asm["dynCall_viiiiiiii"];
var dynCall_viiiiii = Module["dynCall_viiiiii"] = asm["dynCall_viiiiii"];
var dynCall_viii = Module["dynCall_viii"] = asm["dynCall_viii"];
var dynCall_vidddd = Module["dynCall_vidddd"] = asm["dynCall_vidddd"];
var dynCall_vdi = Module["dynCall_vdi"] = asm["dynCall_vdi"];
var dynCall_viiiiiii = Module["dynCall_viiiiiii"] = asm["dynCall_viiiiiii"];
var dynCall_viiiiiiiii = Module["dynCall_viiiiiiiii"] = asm["dynCall_viiiiiiiii"];
var dynCall_iii = Module["dynCall_iii"] = asm["dynCall_iii"];
var dynCall_i = Module["dynCall_i"] = asm["dynCall_i"];
var dynCall_vdddddd = Module["dynCall_vdddddd"] = asm["dynCall_vdddddd"];
var dynCall_vdddd = Module["dynCall_vdddd"] = asm["dynCall_vdddd"];
var dynCall_vdd = Module["dynCall_vdd"] = asm["dynCall_vdd"];
var dynCall_v = Module["dynCall_v"] = asm["dynCall_v"];
var dynCall_viid = Module["dynCall_viid"] = asm["dynCall_viid"];
var dynCall_viiii = Module["dynCall_viiii"] = asm["dynCall_viiii"];
;

Runtime.stackAlloc = Module['stackAlloc'];
Runtime.stackSave = Module['stackSave'];
Runtime.stackRestore = Module['stackRestore'];
Runtime.establishStackSpace = Module['establishStackSpace'];

Runtime.setTempRet0 = Module['setTempRet0'];
Runtime.getTempRet0 = Module['getTempRet0'];



// === Auto-generated postamble setup entry stuff ===

Module['asm'] = asm;





function ExitStatus(status) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + status + ")";
  this.status = status;
};
ExitStatus.prototype = new Error();
ExitStatus.prototype.constructor = ExitStatus;

var initialStackTop;
var preloadStartTime = null;
var calledMain = false;

dependenciesFulfilled = function runCaller() {
  // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
  if (!Module['calledRun']) run();
  if (!Module['calledRun']) dependenciesFulfilled = runCaller; // try this again later, after new deps are fulfilled
}

Module['callMain'] = Module.callMain = function callMain(args) {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on __ATMAIN__)');
  assert(__ATPRERUN__.length == 0, 'cannot call main when preRun functions remain to be called');

  args = args || [];

  ensureInitRuntime();

  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < 4-1; i++) {
      argv.push(0);
    }
  }
  var argv = [allocate(intArrayFromString(Module['thisProgram']), 'i8', ALLOC_NORMAL) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(allocate(intArrayFromString(args[i]), 'i8', ALLOC_NORMAL));
    pad();
  }
  argv.push(0);
  argv = allocate(argv, 'i32', ALLOC_NORMAL);


  try {

    var ret = Module['_main'](argc, argv, 0);


    // if we're not running an evented main loop, it's time to exit
    exit(ret, /* implicit = */ true);
  }
  catch(e) {
    if (e instanceof ExitStatus) {
      // exit() throws this once it's done to make sure execution
      // has been stopped completely
      return;
    } else if (e == 'SimulateInfiniteLoop') {
      // running an evented main loop, don't immediately exit
      Module['noExitRuntime'] = true;
      return;
    } else {
      var toLog = e;
      if (e && typeof e === 'object' && e.stack) {
        toLog = [e, e.stack];
      }
      Module.printErr('exception thrown: ' + toLog);
      Module['quit'](1, e);
    }
  } finally {
    calledMain = true;
  }
}




function run(args) {
  args = args || Module['arguments'];

  if (preloadStartTime === null) preloadStartTime = Date.now();

  if (runDependencies > 0) {
    Module.printErr('run() called, but dependencies remain, so not running');
    return;
  }

  writeStackCookie();

  preRun();

  if (runDependencies > 0) return; // a preRun added a dependency, run will be called later
  if (Module['calledRun']) return; // run may have just been called through dependencies being fulfilled just in this very frame

  function doRun() {
    if (Module['calledRun']) return; // run may have just been called while the async setStatus time below was happening
    Module['calledRun'] = true;

    if (ABORT) return;

    ensureInitRuntime();

    preMain();

    if (ENVIRONMENT_IS_WEB && preloadStartTime !== null) {
      Module.printErr('pre-main prep time: ' + (Date.now() - preloadStartTime) + ' ms');
    }

    if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();

    if (Module['_main'] && shouldRunNow) Module['callMain'](args);

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
  } else {
    doRun();
  }
  checkStackCookie();
}
Module['run'] = Module.run = run;

function exit(status, implicit) {
  if (implicit && Module['noExitRuntime']) {
    Module.printErr('exit(' + status + ') implicitly called by end of main(), but noExitRuntime, so not exiting the runtime (you can use emscripten_force_exit, if you want to force a true shutdown)');
    return;
  }

  if (Module['noExitRuntime']) {
    Module.printErr('exit(' + status + ') called, but noExitRuntime, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)');
  } else {

    ABORT = true;
    EXITSTATUS = status;
    STACKTOP = initialStackTop;

    exitRuntime();

    if (Module['onExit']) Module['onExit'](status);
  }

  if (ENVIRONMENT_IS_NODE) {
    process['exit'](status);
  }
  Module['quit'](status, new ExitStatus(status));
}
Module['exit'] = Module.exit = exit;

var abortDecorators = [];

function abort(what) {
  if (what !== undefined) {
    Module.print(what);
    Module.printErr(what);
    what = JSON.stringify(what)
  } else {
    what = '';
  }

  ABORT = true;
  EXITSTATUS = 1;

  var extra = '';

  var output = 'abort(' + what + ') at ' + stackTrace() + extra;
  if (abortDecorators) {
    abortDecorators.forEach(function(decorator) {
      output = decorator(output, what);
    });
  }
  throw output;
}
Module['abort'] = Module.abort = abort;

// {{PRE_RUN_ADDITIONS}}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

// shouldRunNow refers to calling main(), not run().
var shouldRunNow = true;
if (Module['noInitialRun']) {
  shouldRunNow = false;
}


run();

// {{POST_RUN_ADDITIONS}}





// {{MODULE_ADDITIONS}}



