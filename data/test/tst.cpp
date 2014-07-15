#include <stdio.h>
#include "../Step.h"
#include "../DrumStep.h"
#include "../DummyStepMemory.h"
#include "../FlashStepMemory.h"
#include "../IStepMemory.h"
#include "SRAMHWMock.h"

bool checkMemory(IStepMemory & memory, DrumStep & drumstep, DrumStep & drumstep2) {

    memory.setDrumStep(0, 0, 0, drumstep);
    memory.setDrumStep(13, 7, 61, drumstep2);

    DrumStep drumStepResult = memory.getDrumStep(0, 0, 0);
    DrumStep drumStepResult2 = memory.getDrumStep(13, 7, 61);

    bool memory1OK = 	drumstep.isActive() == drumStepResult.isActive() &&
                        drumstep.isMuted() == drumStepResult.isMuted() &&
                    drumstep.getSubStep(0) == drumStepResult.getSubStep(0) &&
                    drumstep.getSubStep(1) == drumStepResult.getSubStep(1) &&
                    drumstep.getSubStep(2) == drumStepResult.getSubStep(2) &&
                    drumstep.getSubStep(3) == drumStepResult.getSubStep(3);
    bool memory2OK =	drumstep2.isActive() == drumStepResult2.isActive() &&
                        drumstep2.isMuted() == drumStepResult2.isMuted() &&
                        drumstep2.getSubStep(0) == drumStepResult2.getSubStep(0) &&
                        drumstep2.getSubStep(1) == drumStepResult2.getSubStep(1) &&
                        drumstep2.getSubStep(2) == drumStepResult2.getSubStep(2) &&
                        drumstep2.getSubStep(3) == drumStepResult2.getSubStep(3);

    return memory1OK && memory2OK;
}

int main( int argc, const char* argv[] )
{
    Step step = Step(false, false);
    Step step2 = Step(true, true);
    printf("Test Step: %s \n", (!step.isActive() && !step.isMuted() && step2.isActive() && step2.isMuted()) ? "OK" : "NOK");


    DrumStep::DrumVelocityType steps1[4] = {DrumStep::NORMAL, DrumStep::DOWN, DrumStep::UP, DrumStep::NORMAL};
    DrumStep::DrumVelocityType steps2[4] = {DrumStep::DOWN, DrumStep::UP, DrumStep::NORMAL, DrumStep::NORMAL};
    DrumStep drumstep = DrumStep(false, false, steps1);
    DrumStep drumstep2 = DrumStep(true, true, steps2);

    printf("Drum Step standard: %s \n", (!drumstep.isActive() && !drumstep.isMuted() && drumstep2.isActive() && drumstep2.isMuted()) ? "OK" : "NOK");
    printf("Drum Step(1) substeps: %s \n", ((drumstep.getSubStep(0) == steps1[0]) &&
                                            (drumstep.getSubStep(1) == steps1[1]) &&
                                            (drumstep.getSubStep(2) == steps1[2]) &&
                                            (drumstep.getSubStep(3) == steps1[3]) ) ? "OK" : "NOK");
    printf("Drum Step(2) substeps: %s \n", ((drumstep2.getSubStep(0) == steps2[0]) &&
                                            (drumstep2.getSubStep(1) == steps2[1]) &&
                                            (drumstep2.getSubStep(2) == steps2[2]) &&
                                            (drumstep2.getSubStep(3) == steps2[3]) ) ? "OK" : "NOK");

    DummyStepMemory memory = DummyStepMemory();
    printf("Dummy memory check: %s \n", checkMemory(memory, drumstep, drumstep2) ? "OK" : "Error");

    SRAMHWMock hwMOck;
    FlashStepMemory flashMemory = FlashStepMemory(&hwMOck);
    printf("Flash memory check: %s \n", checkMemory(flashMemory, drumstep, drumstep2) ? "OK" : "Error");

    DrumStep resultDrumStep;
    unsigned char nextStep = 0;
    bool nextValueExists = flashMemory.getNextActiveDrumStep(0, 0, nextStep, resultDrumStep);

    printf("Flash memory next valid (next step valids) check: %s \n", nextValueExists && nextStep == 1 ? "OK" : "Error");

    nextStep = 61;
    nextValueExists = flashMemory.getNextActiveDrumStep(13, 7, nextStep, resultDrumStep);

    printf("Flash memory next valid (this step valid) check: %s \n", nextValueExists && nextStep == 61 ? "OK" : "Error");

    for (unsigned char i = 0; i < 64; i++) {
        flashMemory.setDrumStep(1, 1, i, drumstep);
    }

    nextStep = 0;
    nextValueExists = flashMemory.getNextActiveDrumStep(1, 1, nextStep, resultDrumStep);

    printf("Flash memory next valid (no step valid) check: %s \n", !nextValueExists ? "OK" : "Error");

}
