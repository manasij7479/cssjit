#include <map>
namespace cssjit {
class Codegen {
public:
  Codegen(std::map<int, int> data_) : data(data_) {}
  void operator()(std::string filename) {
    //implement
  }
private:
  std::map<int, int> data;
};
}
