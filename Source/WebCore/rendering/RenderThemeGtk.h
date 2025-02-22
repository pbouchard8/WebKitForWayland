/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2006 Apple Inc.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2010 Igalia S.L.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RenderThemeGtk_h
#define RenderThemeGtk_h

#include "RenderTheme.h"

namespace WebCore {

class RenderThemeGtk final : public RenderTheme {
public:
    static Ref<RenderTheme> create();

#if ENABLE(DATALIST_ELEMENT)
    // Returns size of one slider tick mark for a horizontal track.
    // For vertical tracks we rotate it and use it. i.e. Width is always length along the track.
    IntSize sliderTickSize() const override;
    // Returns the distance of slider tick origin from the slider track center.
    int sliderTickOffsetFromTrackCenter() const override;
#endif

private:
    // System fonts.
    void updateCachedSystemFontDescription(CSSValueID, FontCascadeDescription&) const override;

public:
#ifndef GTK_API_VERSION_2

    // A method asking if the theme's controls actually care about redrawing when hovered.
    bool supportsHover(const RenderStyle&) const override { return true; }

    // A method asking if the theme is able to draw the focus ring.
    bool supportsFocusRing(const RenderStyle&) const override;

    // A method asking if the control changes its tint when the window has focus or not.
    bool controlSupportsTints(const RenderObject&) const override;

    // A general method asking if any control tinting is supported at all.
    bool supportsControlTints() const override { return true; }

    void adjustRepaintRect(const RenderObject&, FloatRect&) override;

    // A method to obtain the baseline position for a "leaf" control.  This will only be used if a baseline
    // position cannot be determined by examining child content. Checkboxes and radio buttons are examples of
    // controls that need to do this.
    int baselinePosition(const RenderBox&) const override;

    // The platform selection color.
    Color platformActiveSelectionBackgroundColor() const override;
    Color platformInactiveSelectionBackgroundColor() const override;
    Color platformActiveSelectionForegroundColor() const override;
    Color platformInactiveSelectionForegroundColor() const override;

    // List Box selection color
    Color platformActiveListBoxSelectionBackgroundColor() const override;
    Color platformActiveListBoxSelectionForegroundColor() const override;
    Color platformInactiveListBoxSelectionBackgroundColor() const override;
    Color platformInactiveListBoxSelectionForegroundColor() const override;

    double caretBlinkInterval() const override;

    void platformColorsDidChange() override;

    // System colors.
    Color systemColor(CSSValueID) const override;

    bool popsMenuBySpaceOrReturn() const override { return true; }

#if ENABLE(VIDEO)
    String extraMediaControlsStyleSheet() override;
    String formatMediaControlsCurrentTime(float currentTime, float duration) const override;
    bool supportsClosedCaptioning() const override { return true; }
    String mediaControlsScript() override;

#if ENABLE(FULLSCREEN_API)
    String extraFullScreenStyleSheet() override;
#endif
#endif

    bool shouldHaveCapsLockIndicator(HTMLInputElement&) const override;

private:
    RenderThemeGtk();
    virtual ~RenderThemeGtk();

    bool paintCheckbox(const RenderObject&, const PaintInfo&, const IntRect&) override;
    void setCheckboxSize(RenderStyle&) const override;

    bool paintRadio(const RenderObject&, const PaintInfo&, const IntRect&) override;
    void setRadioSize(RenderStyle&) const override;

    void adjustButtonStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintButton(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustTextFieldStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintTextField(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    bool paintTextArea(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    LengthBox popupInternalPaddingBox(const RenderStyle&) const override;

    // The Mac port differentiates between the "menu list" and the "menu list button."
    // The former is used when a menu list button has been styled. This is used to ensure
    // Aqua themed controls whenever possible. We always want to use GTK+ theming, so
    // we don't maintain this differentiation.
    void adjustMenuListStyle(StyleResolver&, RenderStyle&, Element*) const override;
    void adjustMenuListButtonStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintMenuList(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    bool paintMenuListButtonDecorations(const RenderBox&, const PaintInfo&, const FloatRect&) override;

    void adjustSearchFieldResultsDecorationPartStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintSearchField(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsButtonStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintSearchFieldResultsButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldCancelButtonStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintSearchFieldCancelButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    bool paintSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;
    void adjustSliderTrackStyle(StyleResolver&, RenderStyle&, Element*) const override;

    bool paintSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;
    void adjustSliderThumbStyle(StyleResolver&, RenderStyle&, Element*) const override;

    void adjustSliderThumbSize(RenderStyle&, Element*) const override;

#if ENABLE(VIDEO)
    bool hasOwnDisabledStateHandlingFor(ControlPart) const override;
    bool paintMediaFullscreenButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaPlayButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaMuteButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSeekBackButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSeekForwardButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaVolumeSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaVolumeSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaCurrentTime(const RenderObject&, const PaintInfo&, const IntRect&) override;
#if ENABLE(VIDEO_TRACK)
    bool paintMediaToggleClosedCaptionsButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
#endif
#endif

    double animationRepeatIntervalForProgressBar(RenderProgress&) const override;
    double animationDurationForProgressBar(RenderProgress&) const override;
    void adjustProgressBarStyle(StyleResolver&, RenderStyle&, Element*) const override;
    IntRect progressBarRectForBounds(const RenderObject&, const IntRect&) const override;
    bool paintProgressBar(const RenderObject&, const PaintInfo&, const IntRect&) override;

    InnerSpinButtonLayout innerSpinButtonLayout(const RenderObject&) const override;
    void adjustInnerSpinButtonStyle(StyleResolver&, RenderStyle&, Element*) const override;
    bool paintInnerSpinButton(const RenderObject&, const PaintInfo&, const IntRect&) override;

    String fileListNameForWidth(const FileList*, const FontCascade&, int width, bool multipleFilesAllowed) const override;

    static void setTextInputBorders(RenderStyle&);

#if ENABLE(VIDEO)
    bool paintMediaButton(const RenderObject&, GraphicsContext&, const IntRect&, const char* iconName);
#endif

    static IntRect calculateProgressRect(const RenderObject&, const IntRect&);
#endif // GTK_API_VERSION_2
};

}

#endif // RenderThemeGtk_h
