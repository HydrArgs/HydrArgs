HydrArgs - an abstraction layer around CLI parsing libs
=======================================================

![logo](./resources/logo.svg) <!-- https://user-images.githubusercontent.com/240344/213681687-b20f24d2-3dee-4465-949e-ee2ffc9fdf70.png) -->

Provides an interface for the ones who want to be able to switch CLI parsing libraries easily.

Different CLI parsing libs have different advantages and disadvantages:

* Some of them large and some of them are small;
* Some of them rely heavily on C++ templates, and some of them have API in C;
* Some of them are header-only, and some of them are shared libs;
* Some of them have hard to use API;

And the most importantly:
* They have differrent style of autogenerated options and different users may prefer different libs. It is the reason to allow a **user** to choose the CLI parsing library, not software developer.
* Some of them have cool killer features like generation of autocompletion info or reading args from a config file and environment variables; It is a yet another reason to allow a **user** to choose a library.
* And the most importantly, any of it can be already present on user's or builder's machine. Introducing a dependency means your software cannot be always built. You first have to have the dependency available. This means it has to be properly obtained and packaged. And maybe even brought into the distro you use. If noone has done the work for you, it's you who would have to do the work. Of course, you can vendorize the sources, but it is not a very maintainable approach. And in some cases distro maintainers may require you to properly package the dependencies. This is the reason to **automatically discover libraries in runtime** and use the ones present on a machine.


So we create an abstraction layer having some traits of a hydra:
* multi-headed;
* surviving head amputation.


Usage
-----

The abstraction layer consists of

* some headers defining the interface between backends and software using CLI args parser libs;
* backends. They can be either integrated into the libs, or standalone ones;
* a dispatcher library, discovering backends in runtime and selecting the best backend.

So, the library supports the following use cases:

1. Just linking the backend libraries directly.
2. Linking the intermediate dispatcher library.

And there should be some flavours, which are supported in a CMake importer.

Dispatcher linkage
-------------------

```cmake
find_package(HydrArgs COMPONENT dispatcher)
target_link_libraries(target PRIVATE HydrArgs::dispatcher)
```

Then an end user can customize the backend he wants using env variables.

Backend linkage
---------------

```cmake
find_package(HydrArgs COMPONENT dispatcher)
target_link_libraries(target PRIVATE HydrArgs::backend_<BackendName>)
```


Interface between backend and app
---------------------------------

A backend is just a wrapper around a CLI args parsing library. It is a shared library with 1 static field and 1 function.

The field is `const ParserCapabilities capabilities` and consists of boolean flags defining capabilities of the library.

The function is `IArgsParser* argsParserFactory` which is just a factory returning the pointer to the parser class.

Parser classes must inherit `IArgsParser` and their ctor must accept exactly the same args as `IArgsParser`, and their `operator()` must do the actual parsing.

`operator()` returns `ParseResult`, which describes if the parsing has succeeded correctly, and if the behavior should differ from main app functionality (showing help, auto-completion, etc).


Creation of a backend
---------------------

1. Play around the library a bit to determine its capabilities and limitations.
2. Create an entry in [`backends/CMakeLists.txt`](./backends/CMakeLists.txt) for it. Fill in its capabilities and limitations.
3. Copy the file of the backend the most similar to the one you gonna create.
4. Put the setup code into the ctor
5. Put the code creating an element of an arg spec into `_addArg`.
6. If the parser needs some finalization before it can be used, populate BOTH `_seal` (with the code doing finalization) and `_unseal` (with the code undoing it). If you cannot find the way to undo finalization and it is required to undo it, the library cannot be supported as it is and requires patches.
6. Populate `printHelp` with the code generating help message and outputting it into the streams provided as arguments.
7. Populate `_parseArgs` with the code triggering actual parsing and postprocessing of args.
    * Call the function to trigger args parsing.
    * Check for errors. You can use `PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE_WITH_KNOWN_LOCATION` macro for handling errors.
    * If your impl doesn't have built-in ways of generating error messages, you can use the fallback impls from `HydrArgs::fallback::errors`.
    * Process the case of help message being required to be printed, if it is needed to be done explicitly. You can use `PROCESS_CASE_OF_HELP_CALLED` macro.
    * In any case, when help is requested, the proper result should be returned. You can use `RESULT_HELP_CALLED` symbol.
    * If the library is incapable to handle positional args itself, you can use `HydrArgs::fallback::positional`.
    * In these case check if there is enough positional arguments first. `HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE` macro can be helpful.
    * If the parser lib doesn't populate the results into variables itself, you can need a loop and a large switch.
    * It is likely that you will have to process positional boolean args yourself.
    * If the library is incapable to parse some types of arguments itself, use the provided libs with fallback implementations from `HydrArgs::fallback::parsing`.
    * You should return the rest of args. If the lib stores the rest of args as avector of strings, it may make sense to create a field within the class to store these vector, then to use `std::transform` to get pointers to data to match the return interface.
