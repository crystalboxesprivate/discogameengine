#include <d3d11i/d3d11i.h>
#include <d3d11_1.h>
#include <config/config.h>

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;
#ifdef ENGINE_DEBUG_GRAPHICS
Vector<ComPtr<ID3DUserDefinedAnnotation>> annotations;
#endif

DebugState::DebugState(const char *title) {
#ifdef ENGINE_DEBUG_GRAPHICS
  ComPtr<ID3DUserDefinedAnnotation> pPerf;
  HRESULT hr = device_context->QueryInterface(IID_PPV_ARGS(&pPerf));

  if (FAILED(hr)) {
    assert(false);
    }

  wchar_t converted[100];
  mbstowcs(converted, title, 99);
  pPerf->BeginEvent(converted);

  annotations.push_back(pPerf);
#endif
}

DebugState::~DebugState() {
#ifdef ENGINE_DEBUG_GRAPHICS
  annotations.back()->EndEvent();
  annotations.pop_back();
#endif
}

void DebugState::set_marker(const char *text) {
#ifdef ENGINE_DEBUG_GRAPHICS
  ComPtr<ID3DUserDefinedAnnotation> pPerf;
  HRESULT hr = device_context->QueryInterface(__uuidof(pPerf), reinterpret_cast<void **>(pPerf.Get()));
  if (FAILED(hr))
    return;

  wchar_t converted[100];
  mbstowcs(converted, text, 99);
  pPerf->SetMarker(converted);
#endif
}
} // namespace graphicsinterface