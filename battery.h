namespace battery {

  struct Result {
    float voltage;
    float percent;
  };

  class Monitor {
  public:
    virtual ~Monitor() {}

    virtual Result fetch() = 0;
  };

  class FakeMonitor : public Monitor {
    Result fetch() override;
  };

}

