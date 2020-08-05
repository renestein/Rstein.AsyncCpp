#pragma once

namespace Samples::MapReduceActors
{
  class MapReduceActorRunner
  {
  public:

    MapReduceActorRunner() = default;

    MapReduceActorRunner(const MapReduceActorRunner& other) = delete;
    MapReduceActorRunner(MapReduceActorRunner&& other) noexcept = delete;
    MapReduceActorRunner& operator=(const MapReduceActorRunner& other) = delete;
    MapReduceActorRunner& operator=(MapReduceActorRunner&& other) noexcept = delete;
    ~MapReduceActorRunner() = default;
    void Run();
  
  };

}
