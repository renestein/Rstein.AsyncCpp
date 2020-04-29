#include "CancellationTokenSource.h"

using namespace std;
using namespace RStein::AsyncCpp::Detail;
namespace RStein::AsyncCpp::AsyncPrimitives
{ 
  CancellationTokenSource::CancellationTokenSource() : _sharedState(CtsSharedState::New()),
                                                       _token(_sharedState)
  {
    
  }
                                                       

  void CancellationTokenSource::Cancel() const
  {
    _sharedState->NotifyCanceled();
  }

  bool CancellationTokenSource::IsCancellationRequested() const
  {
    return _sharedState->IsCancellationRequested();
  }

  CancellationToken CancellationTokenSource::Token() const
  {
    return _token;
  }
}
