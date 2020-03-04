#include "CancellationTokenSource.h"

using namespace std;
namespace RStein::AsyncCpp::AsyncPrimitives
{
  CancellationTokenSource::CancellationTokenSourcePtr CancellationTokenSource::Create()
  {
    auto cts = make_shared<CancellationTokenSource>();
    ///cts->Token()->_parent = cts;
    return cts;
  }

  CancellationTokenSource::CancellationTokenSource() : enable_shared_from_this<CancellationTokenSource>(),
                                                       _token(CancellationToken::New()),
                                                       _isCancellationRequested(false)
  {
    
  }
                                                       

  void CancellationTokenSource::Cancel()
  {
    _isCancellationRequested = true;
  }

  bool CancellationTokenSource::IsCancellationRequested() const
  {
    return _isCancellationRequested;
  }

  CancellationToken::CancellationTokenPtr CancellationTokenSource::Token() const
  {
    return _token;
  }
}
