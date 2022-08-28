namespace battery {
  enum State { INIT, MEASURING, DONE };

  struct Result {
    State state;
    float voltage;
    float percent;
  };

  class Task {
  public:
    virtual ~Task() {}

    virtual Result run() = 0;
  };

  class FakeTask : public Task {
    Result run() override;

    Result result;
  };

}

