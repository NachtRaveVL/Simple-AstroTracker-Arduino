/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachment Inlines
*/

#ifndef AstroAttachments_HPP
#define AstroAttachments_HPP

// Attachment implementations intentionally stay small. They are non-owning references,
// leaving object lifetime with the controller/sketch just as the sibling attachment layer does.

template<class TObject>
inline bool operator==(const AstroAttachment<TObject> &lhs, const AstroAttachment<TObject> &rhs)
{
    return lhs.getObject() == rhs.getObject() && lhs.getParentSubIndex() == rhs.getParentSubIndex();
}

template<class TObject>
inline bool operator!=(const AstroAttachment<TObject> &lhs, const AstroAttachment<TObject> &rhs)
{
    return !(lhs == rhs);
}

#endif // /ifndef AstroAttachments_HPP
