#pragma once

#include <agz/factory/factory.h>

AGZ_TRACER_FACTORY_BEGIN

void initialize_post_processor_factory(Factory<PostProcessor> &factory);

AGZ_TRACER_FACTORY_END
