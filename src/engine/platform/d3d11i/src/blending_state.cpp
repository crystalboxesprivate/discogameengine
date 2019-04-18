#include <d3d11i/d3d11i.h>

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;

ComPtr<ID3D11BlendState> m_alphaEnableBlendingState;
ComPtr<ID3D11BlendState> m_alphaDisableBlendingState;

struct D3D11BlendState : public BlendState {
  ComPtr<ID3D11BlendState> blend_state;
};

// Blending constants
const int ZERO = D3D11_BLEND_ZERO;
const int ONE = D3D11_BLEND_ONE;
const int SRC_COLOR = D3D11_BLEND_SRC_COLOR;
const int ONE_MINUS_SRC_COLOR = D3D11_BLEND_INV_SRC_COLOR;
const int DST_COLOR = D3D11_BLEND_DEST_COLOR;
const int ONE_MINUS_DST_COLOR = D3D11_BLEND_INV_DEST_COLOR;
const int SRC_ALPHA = D3D11_BLEND_SRC_ALPHA;
const int ONE_MINUS_SRC_ALPHA = D3D11_BLEND_INV_SRC_ALPHA;
const int DST_ALPHA = D3D11_BLEND_DEST_ALPHA;
const int ONE_MINUS_DST_ALPHA = D3D11_BLEND_INV_DEST_ALPHA;
const int SRC_ALPHA_SATURATE = D3D11_BLEND_SRC_ALPHA_SAT;

const int BM_ADD = D3D11_BLEND_OP_ADD;
const int BM_SUBTRACT = D3D11_BLEND_OP_SUBTRACT;
const int BM_REVERSE_SUBTRACT = D3D11_BLEND_OP_REV_SUBTRACT;
const int BM_MIN = D3D11_BLEND_OP_MIN;
const int BM_MAX = D3D11_BLEND_OP_MAX;

BlendStateRef addBlendState(const int srcFactor, const int destFactor, const int blendMode, const int mask,
                            const bool alphaToCoverage) {
  return addBlendState(srcFactor, destFactor, srcFactor, destFactor, blendMode, blendMode, mask, alphaToCoverage);
}

BlendStateRef addBlendState(const int srcFactorRGB, const int destFactorRGB, const int srcFactorAlpha,
                            const int destFactorAlpha, const int blendModeRGB, const int blendModeAlpha, const int mask,
                            const bool alphaToCoverage) {
  auto blend_state = new D3D11BlendState;
  BOOL blendEnable = srcFactorRGB != D3D11_BLEND_ONE || destFactorRGB != D3D11_BLEND_ZERO ||
                     srcFactorAlpha != D3D11_BLEND_ONE || destFactorAlpha != D3D11_BLEND_ZERO;

  D3D11_BLEND_DESC desc;
  desc.AlphaToCoverageEnable = (BOOL)alphaToCoverage;
  desc.IndependentBlendEnable = FALSE;
  for (int i = 0; i < 8; i++) {
    D3D11_RENDER_TARGET_BLEND_DESC &rt = desc.RenderTarget[i];

    rt.BlendEnable = blendEnable;
    rt.SrcBlend = (D3D11_BLEND)srcFactorRGB;
    rt.DestBlend = (D3D11_BLEND)destFactorRGB;
    rt.BlendOp = (D3D11_BLEND_OP)blendModeAlpha;
    rt.SrcBlendAlpha = (D3D11_BLEND)srcFactorAlpha;
    rt.DestBlendAlpha = (D3D11_BLEND)destFactorAlpha;
    rt.BlendOpAlpha = (D3D11_BLEND_OP)blendModeAlpha;
    rt.RenderTargetWriteMask = mask;
  }

  if (FAILED(device->CreateBlendState(&desc, &blend_state->blend_state))) {
    assert(false && "Couldn't create blendstate");
    return nullptr;
  }

  return BlendStateRef(blend_state);
}

void set_blend_state(BlendStateRef blend_state) {
  D3D11BlendState *blend_state_d3d = (D3D11BlendState *)blend_state.get();
  float bld[] = {0.f, 0.f, 0.f, 0.f};
  device_context->OMSetBlendState(blend_state ? blend_state_d3d->blend_state.Get() : nullptr, bld, ~0);
}

// void set_alpha_blending(bool enabled) {
//   float blendFactor[4];

//   // Setup the blend factor.
//   blendFactor[0] = 0.5f;
//   blendFactor[1] = 0.5f;
//   blendFactor[2] = 0.5f;
//   blendFactor[3] = 0.5f;

//   // Turn off the alpha blending.
//   if (enabled) {
//     device_context->OMSetBlendState(m_alphaEnableBlendingState.Get(), blendFactor, 0xffffffff);
//   } else {
//     device_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
//   }

//   return;
// }

// void create_blend_states() {
//   HRESULT result;
//   // Clear the blend state description.
//   D3D11_BLEND_DESC blendStateDescription;
//   ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
//   // Create an alpha enabled blend state description.
//   blendStateDescription.AlphaToCoverageEnable = TRUE;
//   blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
//   blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
//   blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
//   blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//   blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//   blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
//   blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
//   blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

//   // Create the blend state using the description.
//   result = device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
//   if (FAILED(result)) {
//     assert(false);
//     return;
//   }

//   // Modify the description to create an alpha disabled blend state description.
//   blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

//   // Create the second blend state using the description.
//   result = device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);
//   if (FAILED(result)) {
//     assert(false);
//     return;
//   }

//   return;
// }
} // namespace graphicsinterface