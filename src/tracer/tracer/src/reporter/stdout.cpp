#include <iostream>

#include <agz/tracer/core/reporter.h>
#include <agz/utility/console.h>
#include <agz/utility/time.h>

AGZ_TRACER_BEGIN

class StdOutput : public ProgressReporter
{
    double percent_ = 0;

    time::clock_t clock_;
    double total_seconds_ = 0;
    double last_stage_seconds_ = 0;

    console::progress_bar_f_t pbar_ = console::progress_bar_f_t(80, '=');

public:

    using ProgressReporter::ProgressReporter;

    static std::string description()
    {
        return R"___(
stdout [ProgressReporter]
)___";
    }

    void progress(double percent) override
    {
        if(percent > percent_)
        {
            pbar_.set_percent(static_cast<float>(percent));
            pbar_.display();
            percent_ = percent;
        }
    }

    void message(const std::string &msg) override
    {
        std::cout << "[MSG] " << msg << std::endl;
    }

    void error(const std::string &err) override
    {
        std::cout << "[ERR] " << err << std::endl;
    }

    void begin() override
    {
        total_seconds_ = 0;
    }

    void end() override
    {

    }

    void new_stage() override
    {
        percent_ = 0;
        pbar_.reset_time();
        pbar_.set_percent(0);
        pbar_.display();
        clock_.restart();
    }

    void end_stage() override
    {
        pbar_.done();
        last_stage_seconds_ = clock_.ms() / 1000.0;
        total_seconds_ += last_stage_seconds_;
    }

    double total_seconds() override
    {
        return total_seconds_;
    }

    double last_stage_seconds() override
    {
        return last_stage_seconds_;
    }
};

AGZT_IMPLEMENTATION(ProgressReporter, StdOutput, "stdout")

AGZ_TRACER_END