namespace co2 {

  struct Result {
    int co2;
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

