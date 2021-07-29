#pragma once
#include "api.hpp"

#include "args.hpp"


namespace HydrArgs{
	struct HYDRARGS_API ParseEvents{
		bool helpCalled: 1 = false,  ///< a user has requested help using --help flag
		autoCompleteCalled: 1 = false,  ///< built-in auto-completion functionality has been invoked
		syntaxError: 1 = false;  ///< a user has supplied incorrect CLI args

		/// returns true if I should not call software main functionality
		constexpr operator bool() const {
			return helpCalled || autoCompleteCalled || syntaxError;
		}
	};

	extern const struct ParseEvents EVENTS_NONE HYDRARGS_API;  ///< Convenience constant to report that there were no special failures, everything is OK, software creator should execute his software as usual.
	extern const struct ParseEvents EVENTS_HELP_CALLED HYDRARGS_API;  ///< Help was called, should exit the program and maybe print own additions to help
	extern const struct ParseEvents EVENTS_AUTOCOMPLETE_CALLED HYDRARGS_API;  ///< Auto-completion was called, should exit the program.
	extern const struct ParseEvents EVENTS_SYNTAX_ERROR HYDRARGS_API;  ///< Syntax error has occured in CLI args, should exit the program.

	/// Describes if your custom logic is needed to be called, why it is needed to be called and the return code the app should return.
	struct HYDRARGS_API ParseStatus{
		int returnCode = 0;  ///< Return code provided by either CLI parsing or by the person wrapping it into a backend
		struct ParseEvents events = EVENTS_NONE;  ///< more detailed info about what has happenned

		/*ParseStatus();
		ParseStatus(ParseStatus&);
		ParseStatus(const ParseStatus&);
		ParseStatus(ParseStatus&&);

		ParseStatus(int returnCode);  ///< if it is not 0, then it is assummed to be a syntax error
		ParseStatus(struct ParseEvents events);  ///< if there is a syntax error, it sets the code to EXIT_FAILURE
		ParseStatus(int returnCode, struct ParseEvents events);*/

		/// returns true if I should not call software main functionality
		constexpr operator bool() const {
			return returnCode || events;
		}
	};

	extern const struct ParseStatus STATUS_OK HYDRARGS_API;  ///< Convenience constant to report that there were no special failures, everything is OK, software creator should execute his software as usual.
	extern const struct ParseStatus STATUS_HELP_CALLED HYDRARGS_API;  ///< Help was called, should exit the program and maybe print own additions to help
	extern const struct ParseStatus STATUS_AUTOCOMPLETE_CALLED HYDRARGS_API;  ///< Auto-completion was called, should exit the program.
	extern const struct ParseStatus STATUS_SYNTAX_ERROR HYDRARGS_API;  ///< Syntax error has occured in CLI args, should exit the program.

	struct HYDRARGS_API ParseResult{
		const struct ParseStatus parsingStatus = STATUS_OK;  ///< Describes if your custom logic is needed to be called, why it is needed to be called and the return code the app should return.
		const struct CLIRawArgs rest = NO_ARGS;  ///< Rest of arguments that can be passed to another parser
	};

	extern const struct ParseResult RESULT_HELP_CALLED HYDRARGS_API;  ///< Help was called, should exit the program and maybe print own additions to help
	extern const struct ParseResult RESULT_AUTOCOMPLETE_CALLED HYDRARGS_API;  ///< Auto-completion was called, should exit the program.
	extern const struct ParseResult RESULT_SYNTAX_ERROR HYDRARGS_API;  ///< Syntax error has occured in CLI args, should exit the program.
};
