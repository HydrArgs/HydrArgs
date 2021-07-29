Algorythm of implementing a backend
===================================

The backends metadata are configured within a certain table.

Answer the questions

0. Does the library support separating rest of arguments (especially positional) from the arguments that must be processed by the lib itself?
	* Yea - OK ✅
	* Nay - you cannot wrap such a lib. It provides you not enough data to pass into our fallback implementations. ❌

2. How does the library store results and how does it pass them to you.
	* It stores results itself within itself. We will have to have a loop and dispatching within `_parseArgs`
		* It allows you to create an object, representing the arg and containing its storage. Then access the data via that object. You need to implement dispatching within `_addArg`. In that dispatching you create an object of needed type.
			* The objects are stored and destroyed on the side of creator:
				* Create a storage for them, likely `std::vector`.
				* Emplace them into it.
				* Pass pointers to the emplaced objects to the function adding them into the library.
			* The object are stored and destroyed on the side of the library. Just add them into the needed container. ✅
		* It allows you to specify the arguments in one call and retrieve them by key in an other call on the CLI parser object itself.
	* It puts result by a reference/pointer stored within argument object itself. In this case `_parseArgs` will likely be simple. ✅

3. Does the library support positional arguments?
	* Yea - OK, use its machinery. ✅
	* Nay - we'll use our fallback implementation:
		* Inherit and implement the needed fallback class

			* If the library doesn't store results itself within itself OR if its underlying storage that is continious in memory and stores `char *` is available, then it is easy and efficient to access that storage directly ✅. Inherit and implement `PositionalParserSpannable`
				* get remaining unparsed positional arguments
				* convert them into `std::span<char *>`
				* call `parsePositionals(rawArgs.argv0, restSpan)` It'd return the structure with parsing results.

			* If the library stores results itself within itself and allows you to get access to them via API, but doesn't expose them as a memory buffer of `char *` - then it is more efficient to iterate one-by-one than allocate a vector of `char *` and fill it. ❌ Inherit and implement `PositionalParserConcealed`.
				* Implement `getMandatoryPositionalArgsCount` to return the count of pos args stored in the class itself.
				* Process them in the loop

		* In `_addArg` dispatch on `isPositional`. For positional args call `addPosArg` It may make sense to put dashed arg processing in a separate inline method.
		* In `_parseArgs`
			* call `HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE()` macro after you have parsed the dashed args with the library itself.


4. How does the library allow you to specify type information.
	* You will necessarily have to store the whole original spec within backend if you cannot get it yourself or if it is too complicated: ❌
		* The library provides no access or introspection to the arg spec objects stored within it.
		* The lib doesn't store spec at all.
			* It redoes parsing on each element retrieval. Then you would have to dispatch around lib built-in functions.
			* It allows only to get strings and relyes on you to do parsing. Use `
		* The lib stores type information in a form of an object of a class, inheriting a certain base class. 
		* The lib stores type information within RTTI. ❌

	* You will probably not have to store the original spec within backend if it is easily retrievable by you or if that info is not needed ✅
		* The lib stores type information in a form of a enum value. You probably don't need to store the original spec within backend!
		* The lib stores type information in a form of a pointer to a predefined parsing function.
		* The lib sets the value itself by reference/pointer.

	* You will probably have to store some parts of the original spec if you don't have to store the whole spec and: ❌
		* The library doesn't implement positional arguments parsing. Then you would have to store the info about positional arguments only in a separate storage. Call `addPosArg` in your `_addArg` to do that.
		* The library doesn't supports some types. Including the case, when the library supports positional arguments, but doesn't support positional booleans. Do the following
			* In `_addArg` specify such args as strings.
			* In `_parseArgs` use `PARSE_TYPED_ARG_FROM_STRING` macro;
