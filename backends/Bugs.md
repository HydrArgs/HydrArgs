Bugs in the wrapped libs that must be fixed
===========================================

`DoxygenUtils.cmake`
--------------------

Upstream it into CMake (add into `FindDoxygen.cmake`).


TCLAP
-----

* `OptionalUnlabeledTracker` in `UnlabeledValueArg.h` is a singleton introducing global state rather than the one bound to `CmdLine`. https://sourceforge.net/p/tclap/bugs/34/
It is used to check if ANY positional args are created after ANY positional optional. The check is done in `UnlabeledValueArg` and `UnlabeledMultiArg` ctors. It is completely inacceptable because
	* args are added via `CmdLine::add`, so
	* it prevents from creating multiple `CmdLine`s;
	* It is idiotism. The fact that ANY positional optional arg has been added doesn't mean no more positional args can be added:
		* in fact it is trivial to add an optional positional arg after a yet another optional positional arg given there is a rule to resolve ambiguity. The easiest way to resolve it is to just state that the arg with lower index in the spec first takes precedence over the ones with higher indexes.

argparse
--------

* Non-reentrable. https://github.com/p-ranav/argparse/issues/141

argh
----

* Eats dashed arguments that are out of the spec, so they cannot be passed. https://github.com/adishavit/argh/issues/65

Boost
-----

* it seems that `allow_unregistered` doesn't work. https://github.com/boostorg/program_options/issues/113

Lyra
----

It is unclear how to get rest args. https://github.com/bfgroup/Lyra/discussions/53

dropt
-----

* It doesn't implement positional args https://github.com/jamesderlin/dropt/issues/8
* it doesn't implement mandatory args https://github.com/jamesderlin/dropt/issues/9 (REJECTED!)
* we have some code we can upstream implementing the rudimentary support for non-mandatory positional, mandatory positional and mandatory dashed args.
* It also have very weird API. `.arg_description` must be decoupled from `--aaa=<value>` and `--aaa <value>` support.


argtable
--------

* Upload the fix of the mistake in my packaging (Debug build of the library is not discovered properly)

AnyOption
---------

* Not properly packaged https://github.com/hackorama/AnyOption/pull/32
* Memory safety bugs https://github.com/hackorama/AnyOption/pull/31 (zeroing pointers has mitigated them a bit, but I cannot be sure there are no other such bugs)
* Bad code quality https://github.com/hackorama/AnyOption/pull/38 https://github.com/hackorama/AnyOption/pull/34 https://github.com/hackorama/AnyOption/pull/35 https://github.com/hackorama/AnyOption/pull/36 https://github.com/hackorama/AnyOption/pull/33
* Feels like unmaintained.

Sweet
------

* Wrong order of defining/parsing. It requires argc and argv on construction of the parser, then definition of args, then parsing them.


cxxopts
-------

* `m_positional` is overridden (not appended) on every `app.parse_positional` (this function adds positional arguments). There is no way to only append element there.  https://github.com/jarro2783/cxxopts/issues/300
* There is no way to remove an element from `m_positional_set`.  https://github.com/jarro2783/cxxopts/issues/301

dlib
----

* We cannot reuse the same objects without redoing the most of work on recreating it. https://github.com/davisking/dlib/issues/2421


