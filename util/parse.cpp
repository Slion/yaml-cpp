#include <fstream>
#include <iostream>
#include <vector>

#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

struct Params {
  bool hasFile;
  std::string fileName;
};

Params ParseArgs(int argc, char** argv) {
  Params p;

  std::vector<std::string> args(argv + 1, argv + argc);

  return p;
}

class Anchor {
 public:
  std::string iName;
  std::string iValue;
};

class NullEventHandler : public YAML::EventHandler {
 public:
  int iDepth = 0;
  bool iExpectKey = true;
  bool iIsInSequence = false;
  bool iExpectingAnchorValue = false;
  std::string iKey;
  Anchor iAnchor;
  std::vector<Anchor> iAnchors;
  // std::string iValue;

  void PrintIndent(int aMod = 0) {
    int depth = iDepth + aMod;
    while (depth > 0) {
      depth--;
      std::cout << "   ";
    }
  }

  void OnDocumentStart(const YAML::Mark&) override {}

  void OnDocumentEnd() override { std::cout << "\n\n"; }

  void OnNull(const YAML::Mark& aMark, YAML::anchor_t aAnchor) override {
    std::cout << "Null\n";
  }

  void OnAlias(const YAML::Mark& aMark, YAML::anchor_t aAnchor) override {
    PrintIndent();
    std::cout << iKey << " : *" << iAnchors[aAnchor-1].iValue << "\n";
    iExpectKey = true;
    // std::cout << "Alias\n";
  }

  void OnAnchor(const YAML::Mark& aMark, const std::string& aName) override {
    iExpectingAnchorValue = true;
    iAnchor.iName = aName;
  }

  void OnScalar(const YAML::Mark& aMark, const std::string& aTag,
                YAML::anchor_t aAnchor, const std::string& aValue) override {
    if (iExpectKey && !iIsInSequence) {
      iKey = aValue;
      iExpectKey = false;
    } else if (iIsInSequence) {
      if (!iExpectKey) {
        PrintIndent(-1);
        std::cout << aTag << iKey << " :\n";
      }
      iExpectKey = true;
      PrintIndent();
      std::cout << aTag << " " << aValue << "\n";
    } else {
      iExpectKey = true;

	  if (iExpectingAnchorValue)
	  {
		// Register this anchor
        iAnchor.iValue = aValue;
        iAnchors.push_back(iAnchor);
	  }

      PrintIndent();
      std::cout << aTag << " " << iKey << " : " << aValue << "\n";
    }
  }

  void OnSequenceStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
                       YAML::EmitterStyle::value) override {
    iDepth++;
    iIsInSequence = true;
  }

  void OnSequenceEnd() override {
    iDepth--;
    iIsInSequence = false;
  }

  void OnMapStart(const YAML::Mark& aMark, const std::string& aTag,
                  YAML::anchor_t aAnchor,
                  YAML::EmitterStyle::value aStyle) override {    
    PrintIndent();
    std::cout << aTag << "\n";
    iDepth++;
    iExpectKey = true;
    iIsInSequence = false;
    return;
  }

  void OnMapEnd() override { iDepth--; }
};

void parse(std::istream& input) {

  try {
    // Try our parser
    YAML::Parser parser(input);
    NullEventHandler handler;
    parser.HandleNextDocument(handler);

    // Reset our stream
    input.seekg(std::ios_base::beg);
    YAML::Node doc = YAML::Load(input);
    std::cout << doc << "\n";

  } catch (const YAML::Exception& e) {
    std::cerr << e.what() << "\n";
  }

  do {
    std::cout << '\n' << "Press a key to continue...";
  } while (std::cin.get() != '\n');
}

int main(int argc, char** argv) {
  Params p = ParseArgs(argc, argv);

  if (argc > 1) {
    std::ifstream fin;
    fin.open(argv[1]);
    parse(fin);
  } else {
    parse(std::cin);
  }

  return 0;
}
