// DO NOT EDIT;
// Generated by ml/nn/runtime/test/specs/generate_test.sh
#include "../../TestGenerated.h"

namespace strided_slice_quant8_3 {
std::vector<MixedTypedExample> examples = {
// Generated strided_slice_quant8_3 test
#include "generated/examples/strided_slice_quant8_3.example.cpp"
};
// Generated model constructor
#include "generated/models/strided_slice_quant8_3.model.cpp"
} // namespace strided_slice_quant8_3
TEST_F(GeneratedTests, strided_slice_quant8_3) {
    execute(strided_slice_quant8_3::CreateModel,
            strided_slice_quant8_3::is_ignored,
            strided_slice_quant8_3::examples);
}
