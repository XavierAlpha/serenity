/*
 * Copyright (c) 2021, Idan Horowitz <idan.horowitz@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Runtime/NativeFunction.h>

namespace JS {

class WeakRefConstructor final : public NativeFunction {
    JS_OBJECT(WeakRefConstructor, NativeFunction);

public:
    explicit WeakRefConstructor(GlobalObject&);
    virtual void initialize(GlobalObject&) override;
    virtual ~WeakRefConstructor() override;

    virtual Value call() override;
    virtual Value construct(Function&) override;

private:
    virtual bool has_constructor() const override { return true; }
};

}
