
#pragma once
#ifndef _STDA_SINGLETON_H
#define _STDA_SINGLETON_H

namespace stdA {

    template<class _ST> class Singleton {
        public:
            static _ST& getInstance();

            Singleton(Singleton const& ) = delete;
            Singleton(Singleton&& ) = delete;
            Singleton& operator=(Singleton const& ) = delete;
            Singleton& operator=(Singleton&& ) = delete;

        protected:
            Singleton() {};
            ~Singleton() {};
    };

    template<class _ST> _ST& Singleton<_ST>::getInstance()  {

        static _ST myInstance;

        return myInstance;
    }
}

#endif
