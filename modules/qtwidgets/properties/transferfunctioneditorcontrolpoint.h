/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_TRANSFERFUNCTIONEDITORCONTROLPOINT_H
#define IVW_TRANSFERFUNCTIONEDITORCONTROLPOINT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/transferfunctioneditorprimitive.h>

#include <inviwo/core/datastructures/transferfunctiondatapoint.h>

namespace inviwo {

class TransferFunctionControlPointConnection;

class IVW_MODULE_QTWIDGETS_API TransferFunctionEditorControlPoint
    : public TransferFunctionEditorPrimitive,
      public TransferFunctionPointObserver {
public:
    TransferFunctionEditorControlPoint(TransferFunctionDataPoint* dataPoint, QGraphicsScene* scene,
                                       float size = 14.0f);
    ~TransferFunctionEditorControlPoint() = default;

    virtual void setScalarValue(float scalar) override;
    virtual float getScalarValue() const override;

    virtual void setColor(const vec4& color) override;
    virtual void setColor(const vec3& color) override;
    virtual const vec4& getColor() const override;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + TransferFunctionEditorPrimitive::TFEditorControlPointType };
    int type() const { return Type; }

    virtual void onTransferFunctionPointChange(const TransferFunctionDataPoint* p) override;

    void setDataPoint(TransferFunctionDataPoint* dataPoint);
    TransferFunctionDataPoint* getPoint() const;

    friend IVW_MODULE_QTWIDGETS_API bool operator==(const TransferFunctionEditorControlPoint& lhs,
                                                    const TransferFunctionEditorControlPoint& rhs);

    // Compare points by their "x" value
    friend IVW_MODULE_QTWIDGETS_API bool operator<(const TransferFunctionEditorControlPoint& lhs,
                                                   const TransferFunctionEditorControlPoint& rhs);

    TransferFunctionControlPointConnection* left_ = nullptr;   // Non-owning reference
    TransferFunctionControlPointConnection* right_ = nullptr;  // Non-owning reference

protected:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

private:
    /**
     * draws the primitive. Gets called from within paint()
     *
     * @param painter   painter for drawing the object, pen and brush are set up to match
     *                  primitive color and selection status
     */
    virtual void paintPrimitive(QPainter* painter) override;

    /**
     * Adjust the position prior updating the graphicsitem position considering
     * movement restrictions of the TF point ("restrict" or "push").
     * This function is called in itemChange() before calling onItemPositionChange.
     *
     * Side effects: Neighboring control points will be adjusted in turn.
     *
     * @param pos   candidate for new position (in scene coords)
     * @return modified position which is used instead of pos
     */
    virtual QPointF prepareItemPositionChange(const QPointF& pos) override;

    /**
     * gets called in itemChange() after a position change of the item. The position
     * is already adjusted to lie within the scene bounding box and normalized.
     *
     * @param newPos   new, normalized position of the primitive
     */
    virtual void onItemPositionChange(const vec2& newPos) override;

    /**
     * gets called in itemChange() when a scene change has happend
     */
    virtual void onItemSceneHasChanged() override;

    TransferFunctionDataPoint* dataPoint_;
};

IVW_MODULE_QTWIDGETS_API bool operator==(const TransferFunctionEditorControlPoint& lhs,
                                         const TransferFunctionEditorControlPoint& rhs);
IVW_MODULE_QTWIDGETS_API bool operator!=(const TransferFunctionEditorControlPoint& lhs,
                                         const TransferFunctionEditorControlPoint& rhs);
IVW_MODULE_QTWIDGETS_API bool operator<(const TransferFunctionEditorControlPoint& lhs,
    const TransferFunctionEditorControlPoint& rhs);
IVW_MODULE_QTWIDGETS_API bool operator>(const TransferFunctionEditorControlPoint& lhs,
                                        const TransferFunctionEditorControlPoint& rhs);
IVW_MODULE_QTWIDGETS_API bool operator<=(const TransferFunctionEditorControlPoint& lhs,
                                         const TransferFunctionEditorControlPoint& rhs);
IVW_MODULE_QTWIDGETS_API bool operator>=(const TransferFunctionEditorControlPoint& lhs,
                                         const TransferFunctionEditorControlPoint& rhs);

}  // namespace inviwo

#endif  // IVW_TRANSFERFUNCTIONEDITORCONTROLPOINT_H
