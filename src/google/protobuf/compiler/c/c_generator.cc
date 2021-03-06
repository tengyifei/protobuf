//
// Created by yifeit on 7/7/16.
//

#include <google/protobuf/compiler/c/c_generator.h>

#include <stdlib.h>
#include <iostream>
#include <memory>
#ifndef _SHARED_PTR_H
#include <google/protobuf/stubs/shared_ptr.h>
#endif
#include <vector>

#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

// Returns the list of the names of files in all_files in the form of a
// comma-separated string.
  string CommaSeparatedList(const vector<const FileDescriptor*> all_files) {
    vector<string> names;
    for (int i = 0; i < all_files.size(); i++) {
      names.push_back(all_files[i]->name());
    }
    return Join(names, ",");
  }

  static const char* kFirstInsertionPoint =
    "// @@protoc_insertion_point(first_mock_insertion_point) is here\n";
  static const char* kSecondInsertionPoint =
    "  // @@protoc_insertion_point(second_mock_insertion_point) is here\n";

  CGenerator::CGenerator() {}

  CGenerator::~CGenerator() {}

  bool CGenerator::Generate(
    const FileDescriptor* file,
    const string& parameter,
    GeneratorContext* context,
    string* error) const {
    for (int i = 0; i < file->message_type_count(); i++) {
      if (HasPrefixString(file->message_type(i)->name(), "MockCodeGenerator_")) {
        string command = StripPrefixString(file->message_type(i)->name(),
                                           "MockCodeGenerator_");
        if (command == "Error") {
          *error = "Saw message type MockCodeGenerator_Error.";
          return false;
        } else if (command == "Exit") {
          std::cerr << "Saw message type MockCodeGenerator_Exit." << std::endl;
          exit(123);
        } else if (command == "Abort") {
          std::cerr << "Saw message type MockCodeGenerator_Abort." << std::endl;
          abort();
        } else if (command == "HasSourceCodeInfo") {
          FileDescriptorProto file_descriptor_proto;
          file->CopySourceCodeInfoTo(&file_descriptor_proto);
          bool has_source_code_info =
            file_descriptor_proto.has_source_code_info() &&
            file_descriptor_proto.source_code_info().location_size() > 0;
          std::cerr << "Saw message type MockCodeGenerator_HasSourceCodeInfo: "
          << has_source_code_info << "." << std::endl;
          abort();
        } else if (command == "HasJsonName") {
          FieldDescriptorProto field_descriptor_proto;
          file->message_type(i)->field(0)->CopyTo(&field_descriptor_proto);
          std::cerr << "Saw json_name: "
          << field_descriptor_proto.has_json_name() << std::endl;
          abort();
        } else {
          GOOGLE_LOG(FATAL) << "Unknown MockCodeGenerator command: " << command;
        }
      }
    }

    {
      google::protobuf::scoped_ptr<io::ZeroCopyOutputStream> output(
        context->Open(GetOutputFileName(file) + ".h"));

      io::Printer printer(output.get(), '$');
      printer.PrintRaw(GetOutputFileContent(parameter,
                                            file, context));
      printer.PrintRaw(kFirstInsertionPoint);
      printer.PrintRaw(kSecondInsertionPoint);

      if (printer.failed()) {
        *error = "MockCodeGenerator detected write error.";
        return false;
      }
    }

    {
      google::protobuf::scoped_ptr<io::ZeroCopyOutputStream> output(
        context->Open(GetOutputFileName(file) + ".c"));

      io::Printer printer(output.get(), '$');
      printer.PrintRaw(GetOutputFileContent(parameter,
                                            file, context));
      printer.PrintRaw(kFirstInsertionPoint);
      printer.PrintRaw(kSecondInsertionPoint);

      if (printer.failed()) {
        *error = "MockCodeGenerator detected write error.";
        return false;
      }
    }

    return true;
  }

  inline bool StripSuffix(string *filename, const string &suffix) {
    if (filename->length() >= suffix.length()) {
      size_t suffix_pos = filename->length() - suffix.length();
      if (filename->compare(suffix_pos, string::npos, suffix) == 0) {
        filename->resize(filename->size() - suffix.size());
        return true;
      }
    }

    return false;
  }

  inline string StripProto(string filename) {
    if (!StripSuffix(&filename, ".protodevel")) {
      StripSuffix(&filename, ".proto");
    }
    return filename;
  }

  string CGenerator::GetOutputFileName(const FileDescriptor* file) {
    return GetOutputFileName(file->name());
  }

  string CGenerator::GetOutputFileName(const string& file) {
    return StripProto(file) + ".pbc";
  }

  string CGenerator::GetOutputFileContent(
    const string& parameter,
    const FileDescriptor* file,
    GeneratorContext *context) {
    vector<const FileDescriptor*> all_files;
    context->ListParsedFiles(&all_files);
    return GetOutputFileContent(
      parameter, file->name(),
      CommaSeparatedList(all_files),
      file->message_type_count() > 0 ?
      file->message_type(0)->name() : "(none)");
  }

  string CGenerator::GetOutputFileContent(
    const string& parameter,
    const string& file,
    const string& parsed_file_list,
    const string& first_message_name) {
    return strings::Substitute("// $0, $1, $2, $3\n",
                               parameter, file,
                               first_message_name, parsed_file_list);
  }

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
