Bugs in the wrapped libs that must be fixed
===========================================


**WARNING**: The most of bug info was migrated into `backends.json`. See it first

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
