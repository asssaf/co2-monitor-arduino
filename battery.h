namespace battery {

  struct Result {
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
  };

}

