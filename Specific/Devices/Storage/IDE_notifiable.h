//
// Created by artypoole on 09/09/24.
//

#ifndef IDE_NOTIFIABLE_H
#define IDE_NOTIFIABLE_H



class IDE_notifiable {
public:
    virtual ~IDE_notifiable() = default;
    virtual void notify() = 0;
};



#endif //IDE_NOTIFIABLE_H
