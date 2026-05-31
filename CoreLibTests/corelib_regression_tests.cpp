#include "corelib_regression_tests.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../CoreLib/LibIO.h"
#include "../CoreLib/LibString.h"
#include "../CoreLib/Dictionary.h"
#include "../CoreLib/Link.h"
#include "../CoreLib/List.h"
#include "../CoreLib/SmartPointer.h"
#include "../CoreLib/TextIO.h"
#include "../IL/CFG.h"
#include "../IL/IntermediateCode.h"

namespace
{
	using CoreLib::Basic::Exception;
	using CoreLib::Basic::Dictionary;
	using CoreLib::Basic::HashSet;
	using CoreLib::Basic::KeyValuePair;
	using CoreLib::Basic::LinkedList;
	using CoreLib::Basic::List;
	using CoreLib::Basic::Object;
	using CoreLib::Basic::RefPtr;
	using CoreLib::Basic::String;
	using CoreLib::Basic::StringBuilder;
	using CoreLib::IO::BinaryReader;
	using CoreLib::IO::BinaryWriter;
	using CoreLib::IO::FileAccess;
	using CoreLib::IO::Encoding;
	using CoreLib::IO::FileMode;
	using CoreLib::IO::FileShare;
	using CoreLib::IO::FileStream;
	using CoreLib::IO::Path;
	using CoreLib::IO::StreamWriter;
	using CoreLib::IO::StreamReader;
	using Compiler::Intermediate::ControlFlowNode;
	using Compiler::Intermediate::DataType;
	using Compiler::Intermediate::Function;
	using Compiler::Intermediate::Instruction;
	using Compiler::Intermediate::MemoryLocation;
	using Compiler::Intermediate::MemoryLocationType;
	using Compiler::Intermediate::Operand;
	using Compiler::Intermediate::OperandType;
	using Compiler::Intermediate::Operation;
	using Compiler::Intermediate::Variable;

	class CountingNode : public Object
	{
	public:
		static int DestructorCount;
		~CountingNode() override
		{
			DestructorCount++;
		}
	};

	int CountingNode::DestructorCount = 0;

	void Require(bool condition, const wchar_t* message)
	{
		if (!condition)
			throw std::runtime_error(String(message).ToMultiByteString());
	}

	template<typename TFunc>
	void RequireThrowsOrValue(
		TFunc&& func,
		bool expectedValue,
		const wchar_t* message)
	{
		try
		{
			Require(static_cast<bool>(func()) == expectedValue, message);
		}
		catch (const std::exception&)
		{
		}
		catch (const Exception&)
		{
		}
		catch (const char*)
		{
		}
	}

	template<typename TFunc>
	void RequireThrows(
		TFunc&& func,
		const wchar_t* message)
	{
		bool thrown = false;
		try
		{
			func();
		}
		catch (const std::exception&)
		{
			thrown = true;
		}
		catch (const Exception&)
		{
			thrown = true;
		}
		catch (const char*)
		{
			thrown = true;
		}
		Require(thrown, message);
	}

	void TestStringGuards()
	{
		String value(L"abc");
		RequireThrowsOrValue([&]() { return value.IndexOf(L'a', -1) == -1; }, true, L"String::IndexOf(wchar_t, int) should reject negative start indices.");
		RequireThrowsOrValue([&]() { return value.IndexOf(L"bc", 4) == -1; }, true, L"String::IndexOf(const wchar_t*, int) should reject start indices past the end.");
		RequireThrowsOrValue([&]() { return value.SubString(-1, 1) == String(); }, true, L"String::SubString should reject negative start indices.");
		Require(value.SubString(1, 2) == String(L"bc"), L"String::SubString should keep valid ranges intact.");
		RequireThrowsOrValue([&]() { return value[99] == 0; }, true, L"String::operator[] should reject invalid indices safely.");
		Require(String(L" \t ").Trim() == String(), L"String::Trim should return an explicit empty string for whitespace-only input.");
		Require(String(L"\r\n\t ").Trim() == String(), L"String::Trim should treat newline-style whitespace as trim characters.");
		Require(String(L"\n abc \r").Trim() == String(L"abc"), L"String::Trim should remove newline-style whitespace from both ends.");
		String aliasValue(L"abcd");
		aliasValue = aliasValue.Buffer();
		Require(aliasValue == String(L"abcd"), L"String assignment from its own buffer should preserve the value.");
		aliasValue = aliasValue.Buffer() + 1;
		Require(aliasValue == String(L"bcd"), L"String assignment from an interior pointer of its own buffer should preserve the source text.");
	}

	void TestStringBuilderGuards()
	{
		StringBuilder builder(8);
		Require(builder.Capacity() >= 9, L"StringBuilder should honor the requested initial capacity.");
		builder << L"abcdef";
		builder.Remove(2, 2);
		Require(builder.ToString() == String(L"abef"), L"StringBuilder::Remove should erase the requested range.");
		try
		{
			builder.Remove(-1, 1);
		}
		catch (const char*)
		{
		}
		Require(builder.ToString() == String(L"abef"), L"StringBuilder::Remove should ignore invalid negative indices in release mode.");
		try
		{
			builder.Remove(10, 1);
		}
		catch (const char*)
		{
		}
		Require(builder.ToString() == String(L"abef"), L"StringBuilder::Remove should ignore indices past the end in release mode.");
		try
		{
			builder.Remove(1, -1);
		}
		catch (const char*)
		{
		}
		Require(builder.ToString() == String(L"abef"), L"StringBuilder::Remove should ignore negative lengths in release mode.");
	}

	void TestListGuards()
	{
		List<int> values;
		values.Add(1);
		values.Add(2);
		values.FastRemove(99);
		Require(values.Count() == 2, L"List::FastRemove should not change the count when the value is missing.");
		Require(values[0] == 1 && values[1] == 2, L"List::FastRemove should keep existing contents intact when the value is missing.");
		try
		{
			values.RemoveRange(-1, 1);
		}
		catch (const char*)
		{
		}
		Require(values.Count() == 2, L"List::RemoveRange should ignore negative start indices in release mode.");
		try
		{
			values.RemoveRange(0, -1);
		}
		catch (const char*)
		{
		}
		Require(values.Count() == 2, L"List::RemoveRange should ignore negative delete counts in release mode.");
		try
		{
			values.RemoveRange(10, 1);
		}
		catch (const char*)
		{
		}
		Require(values.Count() == 2, L"List::RemoveRange should ignore out-of-range start indices in release mode.");

		const List<int>& constValues = values;
		Require(constValues.Contains(1) && !constValues.Contains(99), L"List::Contains should be callable on const lists.");

		values = values;
		Require(values.Count() == 2 && values[0] == 1 && values[1] == 2, L"List copy self-assignment should preserve existing contents.");
		values = std::move(values);
		Require(values.Count() == 2 && values[0] == 1 && values[1] == 2, L"List move self-assignment should preserve existing contents.");
	}

	void TestLinkedListGuards()
	{
		LinkedList<int> values;
		values.AddLast(1);
		values.AddLast(2);

		values = values;
		Require(values.Count() == 2 && values.First() == 1 && values.Last() == 2, L"LinkedList copy self-assignment should preserve existing contents.");
		values = std::move(values);
		Require(values.Count() == 2 && values.First() == 1 && values.Last() == 2, L"LinkedList move self-assignment should preserve existing contents.");
		const LinkedList<int>& constValues = values;
		Require(constValues.Count() == 2, L"LinkedList::Count should be callable on const lists.");
	}

	void TestRefPtrGuards()
	{
		class BaseNode : public Object
		{
		public:
			virtual ~BaseNode() = default;
		};

		class DerivedNode : public BaseNode
		{
		};

		class OtherNode : public Object
		{
		};

		RefPtr<BaseNode> emptyBase;
		RefPtr<DerivedNode> emptyDerived;
		emptyDerived = emptyBase;
		Require(emptyDerived == nullptr, L"RefPtr cross-type assignment should keep null sources null.");

		RefPtr<OtherNode> otherNode = new OtherNode();
		RefPtr<DerivedNode> mismatchedDerived;
		mismatchedDerived = otherNode.Ptr();
		Require(mismatchedDerived == nullptr, L"RefPtr assignment from an incompatible raw pointer should stay null.");

		RefPtr<OtherNode> otherNodeShared = new OtherNode();
		RefPtr<DerivedNode> mismatchedShared;
		mismatchedShared = otherNodeShared;
		Require(mismatchedShared == nullptr, L"RefPtr cross-type assignment from an incompatible shared pointer should stay null.");

		CountingNode::DestructorCount = 0;
		RefPtr<CountingNode> countingNode = new CountingNode();
		CountingNode* rawCountingNode = countingNode.Ptr();
		countingNode = rawCountingNode;
		Require(CountingNode::DestructorCount == 0, L"RefPtr assignment from the same raw pointer should not destroy the managed object.");
		countingNode = nullptr;
		Require(CountingNode::DestructorCount == 1, L"RefPtr should still destroy the managed object exactly once after raw self-assignment.");

		CountingNode::DestructorCount = 0;
		RefPtr<CountingNode> detachedNode = new CountingNode();
		detachedNode.Dereferance();
		Require(detachedNode == nullptr, L"RefPtr::Dereferance should detach the current owner after releasing the managed object.");
		detachedNode.Dereferance();
		Require(CountingNode::DestructorCount == 1, L"RefPtr::Dereferance should be safe to call repeatedly after the pointer is detached.");

		CountingNode::DestructorCount = 0;
		RefPtr<CountingNode> sharedFirst = new CountingNode();
		RefPtr<CountingNode> sharedSecond = sharedFirst;
		sharedFirst.Dereferance();
		Require(sharedFirst == nullptr, L"RefPtr::Dereferance should clear the detached shared owner.");
		Require(sharedSecond != nullptr, L"RefPtr::Dereferance should not destroy the object while other shared owners remain.");
		Require(CountingNode::DestructorCount == 0, L"RefPtr::Dereferance should only decrement the reference count when ownership is shared.");
		sharedSecond.Dereferance();
		Require(CountingNode::DestructorCount == 1, L"RefPtr shared detach should still destroy the object exactly once when the last owner detaches.");
	}

	void TestHashSetGuards()
	{
		HashSet<int> values;
		values.Add(1);
		values.Add(2);

		HashSet<int> copy;
		copy = values;
		Require(copy.Count() == 2, L"HashSet copy assignment should preserve the element count.");
		Require(copy.Contains(1) && copy.Contains(2), L"HashSet copy assignment should preserve the stored elements.");

		const HashSet<int>& constCopy = copy;
		Require(constCopy.Contains(1) && constCopy.Contains(2), L"HashSet::Contains should be callable on const sets.");

		int iteratedCount = 0;
		for (auto it = constCopy.begin(); it != constCopy.end(); ++it)
		{
			Require(*it == 1 || *it == 2, L"HashSet const iteration should expose the stored keys.");
			iteratedCount++;
		}
		Require(iteratedCount == 2, L"HashSet const iteration should visit every stored key exactly once.");

		copy = copy;
		Require(copy.Count() == 2 && copy.Contains(1) && copy.Contains(2), L"HashSet copy self-assignment should preserve existing elements.");
		copy = std::move(copy);
		Require(copy.Count() == 2 && copy.Contains(1) && copy.Contains(2), L"HashSet move self-assignment should preserve existing elements.");
	}

	void TestDictionaryGuards()
	{
		Dictionary<int, int> values;
		values.Add(3, 30);
		values.Add(4, 40);

		const Dictionary<int, int>& constValues = values;
		int iteratedCount = 0;
		for (auto it = constValues.begin(); it != constValues.end(); ++it)
		{
			Require(it->Key == 3 || it->Key == 4, L"Dictionary const iteration should expose stored keys.");
			Require(it->Value == 30 || it->Value == 40, L"Dictionary const iteration should expose stored values.");
			iteratedCount++;
		}
		Require(iteratedCount == 2, L"Dictionary const iteration should visit every stored entry exactly once.");

		values = values;
		int value = 0;
		Require(values.Count() == 2 && values.TryGetValue(3, value) && value == 30, L"Dictionary copy self-assignment should preserve existing entries.");
		values = std::move(values);
		value = 0;
		Require(values.Count() == 2 && values.TryGetValue(4, value) && value == 40, L"Dictionary move self-assignment should preserve existing entries.");

		KeyValuePair<std::string, std::string> pair(std::string("key"), std::string("value"));
		pair = std::move(pair);
		Require(pair.Key == "key" && pair.Value == "value", L"KeyValuePair move self-assignment should preserve existing fields.");
	}

	void TestPathHelpers()
	{
		Require(Path::Combine(String(), String(L"foo.txt")) == String(L"foo.txt"), L"Path::Combine should return the non-empty side.");
		Require(Path::ReplaceExt(String(L"folder/file.txt"), L"") == String(L"folder/file"), L"Path::ReplaceExt should remove the extension when given an empty replacement.");
		Require(Path::TruncateExt(String()) == String(), L"Path::TruncateExt should return an explicit empty string for empty input.");
	}

	void TestFileHelpers()
	{
		const auto unicodePath = std::filesystem::temp_directory_path() / L"simplec_corelib_readalltext_selftest.tmp";
		const String unicodeFilePath(unicodePath.c_str());

		FileStream writer(unicodeFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
		const unsigned char utf16leBytes[] = { 0xFF, 0xFE, 0x41, 0x00, 0x42, 0x00 };
		Require(writer.Write(utf16leBytes, static_cast<int>(sizeof(utf16leBytes))) == static_cast<int>(sizeof(utf16leBytes)), L"ReadAllText test file should be written completely.");
		writer.Close();

		Require(CoreLib::IO::File::ReadAllText(unicodeFilePath) == String(L"AB"), L"File::ReadAllText should share StreamReader BOM handling for UTF-16 text.");

		std::error_code ec;
		std::filesystem::remove(unicodePath, ec);
	}

	void TestStreamGuards()
	{
		const auto tempPath = std::filesystem::temp_directory_path() / L"simplec_corelib_selftest.tmp";
		const String filePath(tempPath.c_str());

		{
			FileStream writer(filePath, FileMode::Create, FileAccess::Write, FileShare::None);
			const char bytes[] = "abc";
			Require(writer.Write(bytes, 3) == 3, L"FileStream::Write should write the requested number of bytes.");
			RequireThrows([&]()
			{
				char buffer[4]{};
				static_cast<void>(writer.Read(buffer, 3));
			}, L"Write-only FileStream::Read should fail.");
		}

		{
			FileStream reader(filePath, FileMode::Open, FileAccess::Read, FileShare::None);
			RequireThrows([&]()
			{
				const char bytes[] = "z";
				static_cast<void>(reader.Write(bytes, 1));
			}, L"Read-only FileStream::Write should fail.");
		}

		{
			const auto binaryPath = std::filesystem::temp_directory_path() / L"simplec_corelib_binarywriter_selftest.tmp";
			const String binaryFilePath(binaryPath.c_str());
			RefPtr<CoreLib::IO::Stream> writeStream =
				new FileStream(binaryFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			BinaryWriter writer(writeStream);
			const unsigned short values[] = { 0x1234, 0x5678 };
			writer.Write(values, 2);
			writer.Close();

			RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(binaryFilePath, FileMode::Open, FileAccess::Read, FileShare::None);
			BinaryReader reader(readStream);
			unsigned short readBack[2] = {};
			reader.Read(readBack, 2);
			Require(readBack[0] == values[0] && readBack[1] == values[1], L"BinaryWriter::Write should accept const buffers and preserve the written bytes.");
			readStream->Close();

			std::error_code binaryEc;
			std::filesystem::remove(binaryPath, binaryEc);
		}

		{
			const auto shortBinaryPath = std::filesystem::temp_directory_path() / L"simplec_corelib_binaryreader_short_selftest.tmp";
			const String shortBinaryFilePath(shortBinaryPath.c_str());
			FileStream writer(shortBinaryFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			const unsigned char oneByte[] = { 0x7F };
			Require(writer.Write(oneByte, static_cast<int>(sizeof(oneByte))) == static_cast<int>(sizeof(oneByte)), L"Short BinaryReader test file should be written completely.");
			writer.Close();

			RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(shortBinaryFilePath, FileMode::Open, FileAccess::Read, FileShare::None);
			BinaryReader reader(readStream);
			RequireThrows([&]()
			{
				static_cast<void>(reader.ReadInt32());
			}, L"BinaryReader should reject partial fixed-size reads instead of returning incomplete data.");
			readStream->Close();

			std::error_code shortBinaryEc;
			std::filesystem::remove(shortBinaryPath, shortBinaryEc);
		}

		{
			const auto oversizedStringPath = std::filesystem::temp_directory_path() / L"simplec_corelib_binaryreader_oversized_string_selftest.tmp";
			const String oversizedStringFilePath(oversizedStringPath.c_str());
			RefPtr<CoreLib::IO::Stream> writeStream =
				new FileStream(oversizedStringFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			BinaryWriter writer(writeStream);
			writer.Write(INT_MAX);
			writer.Close();

			RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(oversizedStringFilePath, FileMode::Open, FileAccess::Read, FileShare::None);
			BinaryReader reader(readStream);
			RequireThrows([&]()
			{
				static_cast<void>(reader.ReadString());
			}, L"BinaryReader::ReadString should reject string lengths whose byte size does not fit the stream read contract.");
			readStream->Close();

			std::error_code oversizedStringEc;
			std::filesystem::remove(oversizedStringPath, oversizedStringEc);
		}

		{
			CoreLib::Basic::RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(filePath, FileMode::Open, FileAccess::Read, FileShare::None);
			StreamReader reader(readStream, CoreLib::IO::Encoding::Ansi);
			RequireThrows([&]()
			{
				wchar_t buffer[4]{};
				static_cast<void>(reader.Read(buffer, -1));
			}, L"StreamReader::Read should reject negative lengths.");
		}

		{
			const auto bufferedReadPath = std::filesystem::temp_directory_path() / L"simplec_corelib_streamreader_buffered_read_selftest.tmp";
			const String bufferedReadFilePath(bufferedReadPath.c_str());
			FileStream writer(bufferedReadFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			const char text[] = { 'A', '\n', 'B' };
			Require(writer.Write(text, static_cast<int>(sizeof(text))) == static_cast<int>(sizeof(text)), L"Buffered StreamReader::Read test file should be written completely.");
			writer.Close();

			RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(bufferedReadFilePath, FileMode::Open, FileAccess::Read, FileShare::None);
			StreamReader reader(readStream, CoreLib::IO::Encoding::Ansi);
			wchar_t output[3]{};
			Require(reader.Read(output, 3) == 3, L"StreamReader::Read should read through newlines instead of behaving like ReadLine.");
			Require(output[0] == L'A' && output[1] == L'\n' && output[2] == L'B', L"StreamReader::Read should preserve newline characters in buffered reads.");
			readStream->Close();

			std::error_code bufferedReadEc;
			std::filesystem::remove(bufferedReadPath, bufferedReadEc);
		}

		{
			const auto explicitUnicodePath = std::filesystem::temp_directory_path() / L"simplec_corelib_streamreader_explicit_unicode_selftest.tmp";
			const String explicitUnicodeFilePath(explicitUnicodePath.c_str());
			FileStream writer(explicitUnicodeFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			const unsigned char utf16leBytes[] = { 0x60, 0x4F };
			Require(writer.Write(utf16leBytes, static_cast<int>(sizeof(utf16leBytes))) == static_cast<int>(sizeof(utf16leBytes)), L"Explicit Unicode test file should be written completely.");
			writer.Close();

			RefPtr<CoreLib::IO::Stream> readStream =
				new FileStream(explicitUnicodeFilePath, FileMode::Open, FileAccess::Read, FileShare::None);
			StreamReader reader(readStream, CoreLib::IO::Encoding::Unicode);
			Require(reader.Read() == static_cast<wchar_t>(0x4F60), L"StreamReader should honor explicit Unicode encoding when no BOM or heuristic marker is present.");
			readStream->Close();

			std::error_code explicitUnicodeEc;
			std::filesystem::remove(explicitUnicodePath, explicitUnicodeEc);
		}

		{
			const auto unicodePath = std::filesystem::temp_directory_path() / L"simplec_corelib_unicode_selftest.tmp";
			const String unicodeFilePath(unicodePath.c_str());
			FileStream writer(unicodeFilePath, FileMode::Create, FileAccess::Write, FileShare::None);
			const unsigned char utf16leBytes[] = { 0xFF, 0xFE, 0x41, 0x00 };
			Require(writer.Write(utf16leBytes, static_cast<int>(sizeof(utf16leBytes))) == static_cast<int>(sizeof(utf16leBytes)), L"Unicode test file should be written completely.");
			writer.Close();

			StreamReader reader(unicodeFilePath);
			Require(reader.Peak() == L'A', L"StreamReader::Peak should return the first UTF-16 character after the BOM.");
			Require(reader.Read() == L'A', L"StreamReader should skip a UTF-16 BOM before returning the first character.");

			std::error_code unicodeEc;
			std::filesystem::remove(unicodePath, unicodeEc);
		}

		{
			const auto defaultEncodingPath = std::filesystem::temp_directory_path() / L"simplec_corelib_streamwriter_default_encoding.tmp";
			const String defaultEncodingFilePath(defaultEncodingPath.c_str());
			StreamWriter writer(defaultEncodingFilePath, nullptr);
			writer.Write(L"AB");
			writer.Close();

			StreamReader reader(defaultEncodingFilePath);
			Require(reader.Read() == L'A', L"StreamWriter should fall back to Unicode when constructed with a null encoding.");
			Require(reader.Read() == L'B', L"StreamWriter null-encoding fallback should preserve the remaining text.");

			std::error_code defaultEncodingEc;
			std::filesystem::remove(defaultEncodingPath, defaultEncodingEc);
		}

		{
			const auto unicodeRoundTripPath = std::filesystem::temp_directory_path() / L"simplec_corelib_streamwriter_unicode_roundtrip.tmp";
			const String unicodeRoundTripFilePath(unicodeRoundTripPath.c_str());
			StreamWriter writer(unicodeRoundTripFilePath);
			writer.Write(L"\x4F60");
			writer.Close();

			StreamReader reader(unicodeRoundTripFilePath);
			Require(reader.Read() == static_cast<wchar_t>(0x4F60), L"Path-based StreamWriter should emit a Unicode marker so StreamReader can detect non-ASCII UTF-16 text.");

			std::error_code unicodeRoundTripEc;
			std::filesystem::remove(unicodeRoundTripPath, unicodeRoundTripEc);
		}

		std::error_code ec;
		std::filesystem::remove(tempPath, ec);
	}

	void TestEncodingGuards()
	{
		RequireThrows([&]()
		{
			char bytes[] = "a";
			static_cast<void>(Encoding::Ansi->GetString(bytes, -1));
		}, L"AnsiEncoding::GetString should reject negative lengths.");

		RequireThrows([&]()
		{
			char bytes[] = { 'a', 0, 0, 0 };
			static_cast<void>(Encoding::Unicode->GetString(bytes, -1));
		}, L"UnicodeEncoding::GetString should reject negative lengths.");

		RequireThrows([&]()
		{
			static_cast<void>(Encoding::Ansi->GetString(nullptr, 1));
		}, L"AnsiEncoding::GetString should reject null buffers when length is positive.");

		RequireThrows([&]()
		{
			static_cast<void>(Encoding::Unicode->GetString(nullptr, 2));
		}, L"UnicodeEncoding::GetString should reject null buffers when length is positive.");
	}

	void TestIntermediateMoveSemantics()
	{
		Variable variable;
		variable.Id = 7;
		variable.Version = 3;
		variable.Size = 8;
		variable.Type = DataType::Double;
		variable.Location = MemoryLocation(MemoryLocationType::Register, 2);

		Variable movedVariable;
		movedVariable = std::move(variable);
		Require(movedVariable.Id == 7, L"Variable move assignment should preserve the variable id.");
		Require(movedVariable.Version == 3, L"Variable move assignment should preserve the SSA version.");
		Require(movedVariable.Size == 8, L"Variable move assignment should preserve the storage size.");
		Require(movedVariable.Type == DataType::Double, L"Variable move assignment should preserve the data type.");
		Require(movedVariable.Location == MemoryLocation(MemoryLocationType::Register, 2), L"Variable move assignment should preserve the assigned location.");

		Instruction instruction;
		instruction.Func = Operation::Add;
		instruction.LeftOperand = Operand(1);
		instruction.CFG_Node = reinterpret_cast<Compiler::Intermediate::ControlFlowNode*>(0x1);
		instruction.Operands.push_back(Operand(2));
		instruction.Mark = 1;
		instruction.IsVolatile = 1;

		Instruction movedInstruction;
		movedInstruction = std::move(instruction);
		Require(movedInstruction.Func == Operation::Add, L"Instruction move assignment should preserve the operation.");
		Require(movedInstruction.LeftOperand.IntValue == 1, L"Instruction move assignment should preserve the left operand.");
		Require(movedInstruction.CFG_Node == reinterpret_cast<Compiler::Intermediate::ControlFlowNode*>(0x1), L"Instruction move assignment should preserve the CFG node.");
		Require(movedInstruction.Operands.size() == 1 && movedInstruction.Operands[0].IntValue == 2, L"Instruction move assignment should preserve operands.");
		Require(movedInstruction.Mark != 0, L"Instruction move assignment should preserve the mark flag.");
		Require(movedInstruction.IsVolatile != 0, L"Instruction move assignment should preserve the volatile flag.");

		Function function;
		function.ParameterSize = 12;
		function.VariableSize = 20;

		Function movedFunction;
		movedFunction = std::move(function);
		Require(movedFunction.ParameterSize == 12, L"Function move assignment should preserve parameter size.");
		Require(movedFunction.VariableSize == 20, L"Function move assignment should preserve variable size.");

		ControlFlowNode dominator;
		ControlFlowNode reverseDominator;
		ControlFlowNode child;
		ControlFlowNode reverseChild;
		ControlFlowNode frontier;
		ControlFlowNode reverseFrontier;
		ControlFlowNode node;
		node.Id = 5;
		node.ImmediateDominator = &dominator;
		node.ReverseImmediateDominator = &reverseDominator;
		node.DomChildren.push_back(&child);
		node.ReverseDomChildren.push_back(&reverseChild);
		node.DominateFrontier.push_back(&frontier);
		node.ReverseDominateFrontier.push_back(&reverseFrontier);
		node.LiveOut.SetMax(8);
		node.LiveOut.Add(3);
		node.LiveIn.SetMax(8);
		node.LiveIn.Add(4);

		ControlFlowNode movedNode;
		movedNode = std::move(node);
		Require(movedNode.Id == 5, L"ControlFlowNode move assignment should preserve the node id.");
		Require(movedNode.ImmediateDominator == &dominator, L"ControlFlowNode move assignment should preserve the immediate dominator.");
		Require(movedNode.ReverseImmediateDominator == &reverseDominator, L"ControlFlowNode move assignment should preserve the reverse immediate dominator.");
		Require(movedNode.DomChildren.size() == 1 && movedNode.DomChildren[0] == &child, L"ControlFlowNode move assignment should preserve dominator children.");
		Require(movedNode.ReverseDomChildren.size() == 1 && movedNode.ReverseDomChildren[0] == &reverseChild, L"ControlFlowNode move assignment should preserve reverse dominator children.");
		Require(movedNode.DominateFrontier.size() == 1 && movedNode.DominateFrontier[0] == &frontier, L"ControlFlowNode move assignment should preserve dominate frontier.");
		Require(movedNode.ReverseDominateFrontier.size() == 1 && movedNode.ReverseDominateFrontier[0] == &reverseFrontier, L"ControlFlowNode move assignment should preserve reverse dominate frontier.");
		Require(movedNode.LiveOut.Contains(3), L"ControlFlowNode move assignment should preserve live-out data.");
		Require(movedNode.LiveIn.Contains(4), L"ControlFlowNode move assignment should preserve live-in data.");
	}

	void TestOperationFolding()
	{
		Variable variable;
		variable.Id = 1;
		variable.Type = DataType::Int;
		Variable doubleVariable;
		doubleVariable.Id = 2;
		doubleVariable.Type = DataType::Double;

		Operand result;
		std::vector<Operand> operands;
		operands.push_back(Operand(0));
		operands.push_back(Operand(&variable));
		Require(!Operation::Sub->TryEvaluate(result, operands), L"Operation::Sub should not fold 0 - x into x.");

		operands.clear();
		operands.push_back(Operand(&variable));
		operands.push_back(Operand(1));
		Require(Operation::Mod->TryEvaluate(result, operands), L"Operation::Mod should fold x % 1.");
		Require(result.IsIntegral() && result.IntValue == 0, L"Operation::Mod should fold x % 1 to zero.");

		operands.clear();
		operands.push_back(Operand(1));
		operands.push_back(Operand(&variable));
		Require(!Operation::Mod->TryEvaluate(result, operands), L"Operation::Mod should not fold 1 % x.");

		operands.clear();
		operands.push_back(Operand(1));
		operands.push_back(Operand(2));
		Require(!Operation::Compare->TryEvaluate(result, operands), L"Operation::Compare should not fold as a boolean expression.");

		operands.clear();
		operands.push_back(Operand(&variable));
		operands.push_back(Operand(&variable));
		Require(Operation::Leq->TryEvaluate(result, operands), L"Operation::Leq should fold x <= x for int variables.");
		Require(result.IsIntegral() && result.IntValue == 1, L"Operation::Leq should fold x <= x to true for int variables.");
		Require(Operation::Geq->TryEvaluate(result, operands), L"Operation::Geq should fold x >= x for int variables.");
		Require(result.IsIntegral() && result.IntValue == 1, L"Operation::Geq should fold x >= x to true for int variables.");

		operands.clear();
		operands.push_back(Operand(4.0));
		operands.push_back(Operand(2.0));
		Require(Operation::FDiv->TryEvaluate(result, operands), L"Operation::FDiv should fold non-zero double constants.");
		Require(result.Type == OperandType::ConstDouble && result.DoubleValue == 2.0, L"Operation::FDiv should divide double constants using floating-point values.");

		operands.clear();
		operands.push_back(Operand(&doubleVariable));
		operands.push_back(Operand(&doubleVariable));
		Require(Operation::FSub->TryEvaluate(result, operands), L"Operation::FSub should fold x - x.");
		Require(result.Type == OperandType::ConstDouble && result.DoubleValue == 0.0, L"Operation::FSub should preserve double type when folding double variables.");
		Require(Operation::FDiv->TryEvaluate(result, operands), L"Operation::FDiv should fold x / x.");
		Require(result.Type == OperandType::ConstDouble && result.DoubleValue == 1.0, L"Operation::FDiv should preserve double type when folding double variables.");

		operands.clear();
		operands.push_back(Operand(&doubleVariable));
		operands.push_back(Operand(0.0f));
		Require(Operation::FMul->TryEvaluate(result, operands), L"Operation::FMul should fold multiplication by zero.");
		Require(result.Type == OperandType::ConstDouble && result.DoubleValue == 0.0, L"Operation::FMul should preserve double type when folding double variables.");

		operands.clear();
		operands.push_back(Operand(3.8));
		Require(Operation::D2I->TryEvaluate(result, operands), L"Operation::D2I should fold double constants.");
		Require(result.IsIntegral() && result.IntValue == 3, L"Operation::D2I should truncate double constants to int.");
	}
}

namespace SimpleC
{
	namespace Tests
	{
		int RunCoreLibRegressionTests()
		{
			try
			{
				TestStringGuards();
				TestStringBuilderGuards();
				TestListGuards();
				TestLinkedListGuards();
				TestRefPtrGuards();
				TestHashSetGuards();
				TestDictionaryGuards();
				TestPathHelpers();
				TestFileHelpers();
				TestStreamGuards();
				TestEncodingGuards();
				TestIntermediateMoveSemantics();
				TestOperationFolding();
				std::wcout << L"CoreLib regression tests passed." << std::endl;
				return 0;
			}
			catch (const std::exception& ex)
			{
				std::cerr << "CoreLib regression tests failed: " << ex.what() << std::endl;
				return 1;
			}
			catch (const Exception& ex)
			{
				std::cerr << "CoreLib regression tests failed: " << ex.Message.ToMultiByteString() << std::endl;
				return 1;
			}
			catch (...)
			{
				std::cerr << "CoreLib regression tests failed: unknown exception." << std::endl;
				return 1;
			}
		}
	}
}
