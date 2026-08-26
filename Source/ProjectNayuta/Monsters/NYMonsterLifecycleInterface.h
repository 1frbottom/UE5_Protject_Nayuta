// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NYMonsterLifecycleInterface.generated.h"



class AController;
class ANYMonsterBase;

UINTERFACE(MinimalAPI)
class UNYMonsterLifecycleInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Implemented by whatever owns monster lifetime: Stage returns corpses to its pool, Training
 * resets its single test monster. Lets ANYMonsterBase report a death without knowing which
 * GameMode it is running under, so a new mode never forces an edit to the monster class.
 *
 * Server-only: every call site is already guarded by HasAuthority().
 */
class PROJECTNAYUTA_API INYMonsterLifecycleInterface
{
    GENERATED_BODY()

public:
    /**
     * Server: HP reached zero. Kill count and rewards belong here, and they are granted before
     * the death animation plays. KillerController may be null (environmental/unattributed kill).
     */
    virtual void NotifyMonsterKilled(AController* KillerController, ANYMonsterBase* Monster) {}

    /**
     * Server: the death animation finished and the corpse is ready to be taken back.
     * Return true once the monster has been claimed (pooled or reset); false lets it destroy itself.
     */
    virtual bool ReclaimMonster(ANYMonsterBase* Monster) { return false; }
};
