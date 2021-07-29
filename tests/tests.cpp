#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <HydrArgs/HydrArgs.hpp>

using namespace HydrArgs;
using namespace HydrArgs::Backend;

const char testProgName[] = "shit";
const char testProgramDescription[] = "Our program description";
const char testProgramFallbackUsage[] = "Our program FALLBACK usage";

#if defined(TEST_SPECIAL_HELP) && TEST_SPECIAL_HELP
	TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _special), help){
		const char *argv[]{"--help"};
		CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

		SArg<ArgType::flag> odfA{'a', "odf", "opt dashed flag", 0, "flag", "units", false};
		SArg<ArgType::s4> odiA{'b', "odi", "opt dashed int", 0, "int", "units", 0};
		SArg<ArgType::string> odsA{'c', "ods", "opt dashed str", 0, "string", "units", ""};

		SArg<ArgType::flag> mdfA{'d', "mdf", "mand dashed flag", 1, "flag", "units", false};
		SArg<ArgType::s4> mdiA{'e', "mdi", "mand dashed int", 1, "int", "units", 0};
		SArg<ArgType::string> mdsA{'f', "mds", "mand dashed str", 1, "string", "units", ""};

		SArg<ArgType::flag> mpfA{'g', "mpf", "mand pos flag", 1, "flag", "units", false};
		SArg<ArgType::s4> mpiA{'i', "mpi", "mand pos int", 1, "int", "units", 0};
		SArg<ArgType::string> mpsA{'j', "mps", "mand pos str", 1, "string", "units", ""};

		SArg<ArgType::flag> opfA{'k', "opf", "opt pos flag", 0, "flag", "units", false};
		SArg<ArgType::s4> opiA{'l', "opi", "opt pos int", 0, "int", "units", 0};
		SArg<ArgType::string> opsA{'m', "ops", "opt pos str", 0, "string", "units", ""};

		const std::vector<Arg*> dashedSpec{
			&odfA, &odiA, &odsA,
			&mdfA, &mdiA, &mdsA
		};

		const std::vector<Arg*> positionalSpec{
			&mpfA, &mpiA, &mpsA,
			&opfA, &opiA, &opsA,
		};

		std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, dashedSpec, positionalSpec)};
		auto res = (*ap)(args);
		ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);

		ASSERT_TRUE(res.parsingStatus.events.helpCalled);
		ASSERT_FALSE(res.parsingStatus.events.autoCompleteCalled);
		ASSERT_FALSE(res.parsingStatus.events.syntaxError);
	}
#endif

#if defined(TEST_SPECIAL_CHAINING) && TEST_SPECIAL_CHAINING
	TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _special), chaining){
		const char *argv[]{"--ods", "ods", "--odf", "1", "aaa", "10", "-b", "--bbb"};
		CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

		SArg<ArgType::string> odsA{'c', "ods", "opt dashed str", 0, "string", "units", ""};
		SArg<ArgType::flag> odfA{'a', "odf", "opt dashed flag", 0, "flag", "units", false};
		SArg<ArgType::s4> mpiA{'i', "mpi", "mand pos int", 1, "int", "units", 0};

		std::vector<Arg*> dashedSpec{&odfA, &odsA};
		std::vector<Arg*> positionalSpec{&mpiA};

		std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, dashedSpec, positionalSpec)};
		const auto res = (*ap)(args);
		ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);

		ASSERT_EQ(res.rest.argvRest.size(), 4);

		//testProgName, "aaa", "10", "-b", "--bbb"
		//ASSERT_EQ(res.rest.argv, argv[]);
	}
#endif

#if defined(TEST_DASHEDNESS_DASHED) && TEST_DASHEDNESS_DASHED
	#if defined(TEST_OPTIONALITY_OPTIONAL) && TEST_OPTIONALITY_OPTIONAL
		#if defined(TEST_DATA_TYPE_STRING) && TEST_DATA_TYPE_STRING
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Dashed), string){
				const char *argv[]{"--ods", "s"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::string> odsA{'s', "ods", "descriptionA", 0, "string", "units", ""};
				ap->addArg(&odsA, false);
				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(odsA.value, "s");
			}
		#endif

		#if defined(TEST_DATA_TYPE_INT) && TEST_DATA_TYPE_INT
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Dashed), int){
				const char *argv[]{"--odi", "1"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::s4> odiA{'i', "odi", "descriptionA", 0, "int", "units", 0};
				ap->addArg(&odiA, false);
				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(odiA.value, 1);
			}
		#endif

		#if defined(TEST_DATA_TYPE_DOUBLE) && TEST_DATA_TYPE_DOUBLE
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Dashed), double){
				const char *argv[]{"--odd", "1.5"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::f8> oddA{'d', "odd", "descriptionA", 0, "int", "units", 0};
				ap->addArg(&oddA, false);
				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_NEAR(oddA.value, 1.5, 0.0001);
			}
		#endif

		#if defined(TEST_DATA_TYPE_BOOL) && TEST_DATA_TYPE_BOOL
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Dashed), bool){
				const char *argv[]{"--aaa"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::flag> odf_aA{'a', "aaa", "descriptionA", 0, false};
				ap->addArg(&odf_aA, false);
				SArg<ArgType::flag> odf_bA{'b', "bbb", "crap", 0, false};
				ap->addArg(&odf_bA, false);

				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(odf_aA.value, true);
				ASSERT_EQ(odf_bA.value, false);
			}
		#endif
	#endif

	#if defined(TEST_OPTIONALITY_MANDATORY) && TEST_OPTIONALITY_MANDATORY
		#if defined(TEST_DATA_TYPE_STRING) && TEST_DATA_TYPE_STRING
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Dashed), string){
				const char testValue[] = "csada";
				const char *argv[]{"--aaa", testValue};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::string> mds_aA{'a', "aaa", "descriptionA", 1, ""};
				ap->addArg(&mds_aA, false);
				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_FALSE(res.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res.parsingStatus.events.autoCompleteCalled);
				ASSERT_FALSE(res.parsingStatus.events.syntaxError);
				ASSERT_EQ(mds_aA.value, testValue);

				SArg<ArgType::string> bbb_bA{'b', "bbb", "crap", 1, ""};
				ap->addArg(&bbb_bA, false);

				const auto res2 = (*ap)(args);
				ASSERT_NE(res2.parsingStatus.returnCode, EXIT_SUCCESS);  // NOT EQUAL (NE) !!!
				ASSERT_FALSE(res2.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res2.parsingStatus.events.autoCompleteCalled);
				ASSERT_TRUE(res2.parsingStatus.events.syntaxError);

				ASSERT_EQ(mds_aA.value, testValue);
				ASSERT_EQ(bbb_bA.value, "");
			}
		#endif

		#if defined(TEST_DATA_TYPE_INT) && TEST_DATA_TYPE_INT
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Dashed), int){
				const char *argv[]{"--aaa", "100500"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::s4> mdi_aA{'a', "aaa", "descriptionA", 1, 0};
				ap->addArg(&mdi_aA, false);
				ParseResult const res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);
				ASSERT_FALSE(res.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res.parsingStatus.events.autoCompleteCalled);
				ASSERT_FALSE(res.parsingStatus.events.syntaxError);
				ASSERT_EQ(mdi_aA.value, 100500);

				SArg<ArgType::s4> mdi_bA{'b', "bbb", "crap", 1, 0};
				ap->addArg(&mdi_bA, false);

				const auto res2 = (*ap)(args);
				ASSERT_NE(res2.parsingStatus.returnCode, EXIT_SUCCESS);  // NOT EQUAL (NE) !!!
				ASSERT_FALSE(res2.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res2.parsingStatus.events.autoCompleteCalled);
				ASSERT_TRUE(res2.parsingStatus.events.syntaxError);

				ASSERT_EQ(mdi_aA.value, 100500);
				ASSERT_EQ(mdi_bA.value, 0);
			}
		#endif

		#if defined(TEST_DATA_TYPE_DOUBLE) && TEST_DATA_TYPE_DOUBLE
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Dashed), double){
				const char *argv[]{"--aaa", "1.5"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::f8> mdi_aA{'a', "aaa", "descriptionA", 1, 0};
				ap->addArg(&mdi_aA, false);
				ParseResult const res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);
				ASSERT_FALSE(res.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res.parsingStatus.events.autoCompleteCalled);
				ASSERT_FALSE(res.parsingStatus.events.syntaxError);
				ASSERT_NEAR(mdi_aA.value, 1.5, 0.0001);

				SArg<ArgType::f8> mdi_bA{'b', "bbb", "crap", 1, 9.8};
				ap->addArg(&mdi_bA, false);

				const auto res2 = (*ap)(args);
				ASSERT_NE(res2.parsingStatus.returnCode, EXIT_SUCCESS);  // NOT EQUAL (NE) !!!
				ASSERT_FALSE(res2.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res2.parsingStatus.events.autoCompleteCalled);
				ASSERT_TRUE(res2.parsingStatus.events.syntaxError);

				ASSERT_NEAR(mdi_aA.value, 1.5, 0.0001);
				ASSERT_EQ(mdi_bA.value, 9.8);
			}
		#endif

		#if defined(TEST_DATA_TYPE_BOOL) && TEST_DATA_TYPE_BOOL
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Dashed), bool){
				const char *argv[]{"--aaa"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::flag> mdf_aA{'a', "aaa", "descriptionA", 1, false};
				ap->addArg(&mdf_aA, false);
				ParseResult const res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);
				ASSERT_FALSE(res.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res.parsingStatus.events.autoCompleteCalled);
				ASSERT_FALSE(res.parsingStatus.events.syntaxError);
				ASSERT_EQ(mdf_aA.value, true);

				SArg<ArgType::flag> mdf_bA{'b', "bbb", "crap", 1, false};
				ap->addArg(&mdf_bA, false);

				const auto res2 = (*ap)(args);
				ASSERT_NE(res2.parsingStatus.returnCode, EXIT_SUCCESS);  // NOT EQUAL (NE) !!!
				ASSERT_FALSE(res2.parsingStatus.events.helpCalled);
				ASSERT_FALSE(res2.parsingStatus.events.autoCompleteCalled);
				ASSERT_TRUE(res2.parsingStatus.events.syntaxError);

				ASSERT_EQ(mdf_aA.value, true);
				ASSERT_EQ(mdf_bA.value, false);
			}
		#endif
	#endif
#endif

#if defined(TEST_DASHEDNESS_POSITIONAL) && TEST_DASHEDNESS_POSITIONAL

	#if defined(TEST_OPTIONALITY_OPTIONAL) && TEST_OPTIONALITY_OPTIONAL
		#if defined(TEST_DATA_TYPE_STRING) && TEST_DATA_TYPE_STRING
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Positional), string){
				const char *argv[]{"s"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::string> ops_aA{'a', "aaa", "descriptionA", 0, ""};
				ap->addArg(&ops_aA, true);
				SArg<ArgType::string> ops_bA{'b', "bbb", "crap", 0, ""};
				ap->addArg(&ops_bA, true);

				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(ops_aA.value, "s");
				ASSERT_EQ(ops_bA.value, "");
			}
		#endif
		#if defined(TEST_DATA_TYPE_INT) && TEST_DATA_TYPE_INT
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Positional), int){
				const char *argv[]{"1"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::s4> opi_aA{'a', "aaa", "descriptionA", 0, 0};
				ap->addArg(&opi_aA, true);
				SArg<ArgType::s4> opi_bA{'b', "bbb", "crap", 0, 0};
				ap->addArg(&opi_bA, true);

				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(opi_aA.value, 1);
				ASSERT_EQ(opi_bA.value, 0);
			}
		#endif
		#if defined(TEST_DATA_TYPE_BOOL) && TEST_DATA_TYPE_BOOL
			TEST(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _NonMandatory_Positional), bool){
				const char *argv[]{"1", "0"};
				CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};

				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::flag> opf_aA{'a', "aaa", "descriptionA", 0, false};
				ap->addArg(&opf_aA, true);
				SArg<ArgType::flag> opf_bA{'b', "bbb", "crap", 0, false};
				ap->addArg(&opf_bA, true);
				SArg<ArgType::flag> opf_cA{'c', "ccc", "crap", 0, false};
				ap->addArg(&opf_cA, true);

				const auto res = (*ap)(args);
				ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
				ASSERT_EQ(res.rest.argvRest.size(), 0);

				ASSERT_EQ(opf_aA.value, true);
				ASSERT_EQ(opf_bA.value, false);
				ASSERT_EQ(opf_cA.value, false);
			}
		#endif
	#endif

	#if defined(TEST_OPTIONALITY_MANDATORY) && TEST_OPTIONALITY_MANDATORY
		#if defined(TEST_DATA_TYPE_STRING) && TEST_DATA_TYPE_STRING
			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _not_enough), string){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::string> ops_aA{'a', "aaa", "descriptionA", 1, ""};
				ap->addArg(&ops_aA, true);
				SArg<ArgType::string> ops_bA{'b', "bbb", "crap", 1, ""};
				ap->addArg(&ops_bA, true);

				{
					const char *argv[]{"s"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					auto res = (*ap)(args);
					ASSERT_NE(res.parsingStatus.returnCode, EXIT_SUCCESS);
				}
			}
			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _enough), string){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::string> ops_aA{'a', "aaa", "descriptionA", 1, ""};
				ap->addArg(&ops_aA, true);
				SArg<ArgType::string> ops_bA{'b', "bbb", "crap", 1, ""};
				ap->addArg(&ops_bA, true);

				{
					const char *argv[]{"s", "tr"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					auto res = (*ap)(args);
					ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
					ASSERT_EQ(res.rest.argvRest.size(), 0);
					ASSERT_EQ(ops_aA.value, "s");
					ASSERT_EQ(ops_bA.value, "tr");
				}
			}
		#endif

		#if defined(TEST_DATA_TYPE_INT) && TEST_DATA_TYPE_INT
			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _not_enough), int){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::s4> opi_aA{'a', "aaa", "descriptionA", 1, 0};
				ap->addArg(&opi_aA, true);
				SArg<ArgType::s4> opi_bA{'b', "bbb", "crap", 1, 0};
				ap->addArg(&opi_bA, true);

				{
					const char *argv[]{"1"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					const auto res = (*ap)(args);
					ASSERT_NE(res.parsingStatus.returnCode, EXIT_SUCCESS);
				}
			}

			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _enough), int){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::s4> opi_aA{'a', "aaa", "descriptionA", 1, 0};
				ap->addArg(&opi_aA, true);
				SArg<ArgType::s4> opi_bA{'b', "bbb", "crap", 1, 0};
				ap->addArg(&opi_bA, true);

				{
					const char *argv[]{"2", "3"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					const auto res = (*ap)(args);
					ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
					ASSERT_EQ(res.rest.argvRest.size(), 0);
					ASSERT_EQ(opi_aA.value, 2);
					ASSERT_EQ(opi_bA.value, 3);
				}
			}
		#endif

		#if defined(TEST_DATA_TYPE_BOOL) && TEST_DATA_TYPE_BOOL
			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _not_enough), bool){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::flag> opf_aA{'a', "aaa", "descriptionA", 1, false};
				ap->addArg(&opf_aA, true);
				SArg<ArgType::flag> opf_bA{'b', "bbb", "crap", 1, false};
				ap->addArg(&opf_bA, true);

				{
					const char *argv[]{"1"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					const auto res = (*ap)(args);
					ASSERT_NE(res.parsingStatus.returnCode, EXIT_SUCCESS);
				}
			}
			TEST(GTEST_CONCAT_TOKEN_(GTEST_CONCAT_TOKEN_(BACKEND_NAME, _Mandatory_Positional), _enough), bool){
				std::unique_ptr<IArgsParser> ap{argsParserFactory(testProgName, testProgramDescription, testProgramFallbackUsage, {}, {})};

				SArg<ArgType::flag> opf_aA{'a', "aaa", "descriptionA", 1, false};
				ap->addArg(&opf_aA, true);
				SArg<ArgType::flag> opf_bA{'b', "bbb", "crap", 1, false};
				ap->addArg(&opf_bA, true);

				{
					const char *argv[]{"1", "0"};
					CLIRawArgs const args{.argv0=testProgName, .argvRest = argv};
					const auto res = (*ap)(args);
					ASSERT_EQ(res.parsingStatus.returnCode, EXIT_SUCCESS);
					ASSERT_EQ(res.rest.argvRest.size(), 0);

					ASSERT_EQ(opf_aA.value, true);
					ASSERT_EQ(opf_bA.value, false);
				}
			}
		#endif
	#endif
#endif
