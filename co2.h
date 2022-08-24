namespace co2 {
  enum State { INIT, MEASURING, DONE };

  struct Result {
    State state;
    int co2;
  };

  class Task {
  public:
    virtual ~Task() {}

    virtual Result run() = 0;
  };

  class FakeTask : public Task {
    Result run() override;

    State state;
  };

}

