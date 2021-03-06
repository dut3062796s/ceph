// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_WATCHER_NOTIFIER_H
#define CEPH_LIBRBD_WATCHER_NOTIFIER_H

#include "include/int_types.h"
#include "include/buffer_fwd.h"
#include "include/Context.h"
#include "include/rados/librados.hpp"
#include "common/Mutex.h"
#include "common/WorkQueue.h"
#include <list>

namespace librbd {

namespace watcher {

class Notifier {
public:
  static const uint64_t NOTIFY_TIMEOUT;

  Notifier(ContextWQ *work_queue, librados::IoCtx &ioctx,
           const std::string &oid);
  ~Notifier();

  void flush(Context *on_finish);
  void notify(bufferlist &bl, bufferlist *out_bl, Context *on_finish);

private:
  typedef std::list<Context*> Contexts;

  struct C_AioNotify : public Context {
    Notifier *notifier;
    Context *on_finish;

    C_AioNotify(Notifier *notifier, Context *on_finish)
      : notifier(notifier), on_finish(on_finish) {
    }
    virtual void finish(int r) override {
      notifier->handle_notify(r, on_finish);
    }
  };

  ContextWQ *m_work_queue;
  librados::IoCtx m_ioctx;
  CephContext *m_cct;
  std::string m_oid;

  Mutex m_aio_notify_lock;
  size_t m_pending_aio_notifies = 0;
  Contexts m_aio_notify_flush_ctxs;

  void handle_notify(int r, Context *on_finish);

};

} // namespace watcher
} // namespace librbd

#endif // CEPH_LIBRBD_WATCHER_NOTIFIER_H
