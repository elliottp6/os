#pragma once

#pragma once

// note: we should be using APIC, but hey, we don't have all the time in the world for a hobby project, now do we?

void pic_enable_irqs();
void pic_disable_irqs();
void pic_acknowledge_irq();
void pic_remap_and_enable_irqs();
